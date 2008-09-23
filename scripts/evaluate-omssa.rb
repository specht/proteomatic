require 'include/proteomatic'
require 'include/fastercsv'
require 'include/misc'
require 'set'
require 'yaml'

class EvaluateOmssa < ProteomaticScript
	def stddev(ak_Values)
		lf_Mean = 0.0
		ak_Values.each { |f| lf_Mean += f.to_f }
		lf_Mean /= ak_Values.size
		
		lf_Sum = 0.0
		ak_Values.each { |f| lf_Sum += (f.to_f - lf_Mean) * (f.to_f - lf_Mean) }
		return Math.sqrt(lf_Sum / ak_Values.size)
	end
	
	def evaluateFiles(ak_Files)
		lk_ScanHash = Hash.new
		#MT_HydACPAN_25_020507:
		#  MT_HydACPAN_25_020507.1058.1058.2.dta:
		#    e: 3.88761e-07
		#    peptides:
		#      NLLAALNHEETR: true
		#    deflines:
		#    - target_proteins.finalModelsV2.fasta;171789
		#    - target_proteins.frozen_GeneCatalog_2007_09_13.fasta;jgi|Chlre3|194475
		#    - target_gpf-peptides.fasta;NLLAALNHEETR
		#    (added later:)
		#    decoy: false
		
		puts "Evaluating #{ak_Files.collect { |x| File::basename(x) }.join(', ')}..."
		
		ak_Files.each do |ls_Filename|
			ls_Spot = File::basename(ls_Filename).split('.').first
			lk_File = File.open(ls_Filename, 'r')
			
			# skip header
			lk_File.readline
			
			lk_File.each do |ls_Line|
				lk_Line = ls_Line.parse_csv()
				ls_Scan = lk_Line[1]
				ls_Peptide = lk_Line[2]
				lf_E = lk_Line[3].to_f
				ls_DefLine = lk_Line[9]
				lf_Mass = lk_Line[4]
				li_Charge = lk_Line[11].to_i
				
				# correct charge in scan name (because when there are multiple charges,
				# only one version of the spectrum may have been sent to OMSSA, because
				# it obviously doens't have to believe the input file, and in that case
				# the charge in the dta filename must be corrected)
				lk_ScanParts = ls_Scan.split('.')
				lk_ScanParts[3] = li_Charge.to_s
				ls_Scan = lk_ScanParts.join('.')
				ls_Spot = lk_ScanParts.first
				lk_ScanHash[ls_Spot] ||= Hash.new
				
				lk_ScanHash[ls_Spot][ls_Scan] = Hash.new if !lk_ScanHash[ls_Spot].has_key?(ls_Scan)
				if (!lk_ScanHash[ls_Spot][ls_Scan].has_key?('e') || lf_E < lk_ScanHash[ls_Spot][ls_Scan]['e'])
					# clear scan hash
					lk_ScanHash[ls_Spot][ls_Scan]['peptides'] = {ls_Peptide => lf_Mass}
					lk_ScanHash[ls_Spot][ls_Scan]['deflines'] = [ls_DefLine]
					lk_ScanHash[ls_Spot][ls_Scan]['e'] = lf_E;
				elsif (lf_E == lk_ScanHash[ls_Spot][ls_Scan]['e'])
					lk_ScanHash[ls_Spot][ls_Scan]['peptides'][ls_Peptide] = lf_Mass
					lk_ScanHash[ls_Spot][ls_Scan]['deflines'].push(ls_DefLine)
				end
			end
		end
		
		# mark decoy scans 
		lk_ScanHash.keys.each do |ls_Spot|
			lk_ScanHash[ls_Spot].keys.each do |ls_Scan|
				lb_Decoy = false
				lk_ScanHash[ls_Spot][ls_Scan]['deflines'].each { |ls_DefLine| lb_Decoy = true if ls_DefLine[0, 6] == 'decoy_' }
				lk_ScanHash[ls_Spot][ls_Scan]['decoy'] = lb_Decoy
			end
		end
		
		li_TotalScanCount = 0
		lk_GoodScans = Array.new
		
		lk_EThresholds = Hash.new
		
		# determine e-value cutoff for each spot
		lk_ScanHash.keys.each do |ls_Spot|
			li_TotalScanCount += lk_ScanHash[ls_Spot].size
			# sort scans by e value
			lk_ScansByE = lk_ScanHash[ls_Spot].keys.sort { |a, b| lk_ScanHash[ls_Spot][a]['e'] <=> lk_ScanHash[ls_Spot][b]['e'] }
			
			li_TotalCount = 0
			li_DecoyCount = 0
			li_CropCount = 0
			lk_ScansByE.each do |ls_Scan|
				li_TotalCount += 1
				li_DecoyCount += 1 if lk_ScanHash[ls_Spot][ls_Scan]['decoy']
				lf_Fpr = li_DecoyCount.to_f * 2.0 / li_TotalCount.to_f * 100.0
				li_CropCount = li_TotalCount if lf_Fpr < @param[:targetFpr]
			end
			
			lk_GoodScans += lk_ScansByE[0, li_CropCount]
			lk_EThresholds[ls_Spot] = lk_ScanHash[ls_Spot][lk_ScansByE[li_CropCount - 1]]['e'] if li_CropCount > 0
		end
		
		# chuck spots out of lk_ScanHash
		lk_NewScanHash = Hash.new
		lk_ScanHash.keys.each { |ls_Spot| lk_NewScanHash.merge!(lk_ScanHash[ls_Spot]) }
		lk_ScanHash = lk_NewScanHash
		
		lk_GoodScans.delete_if { |ls_Scan| lk_ScanHash[ls_Scan]['decoy'] || lk_ScanHash[ls_Scan]['peptides'].size > 1 }
		
		lk_PeptideHash = Hash.new
		#WLQYSEVIHAR:
		#  scans: [MT_HydACPAN_1_300407.100.100.2, ...]
		#  spots: (MT_HydACPAN_1_300407) (set)
		#  found: {gpf, models}
		#  proteins: {x => true, y => true}
		
		lk_GoodScans.each do |ls_Scan|
			ls_Peptide = lk_ScanHash[ls_Scan]['peptides'].keys.first
			if !lk_PeptideHash.has_key?(ls_Peptide)
				lk_PeptideHash[ls_Peptide] = Hash.new 
				lk_PeptideHash[ls_Peptide]['scans'] = Array.new
				lk_PeptideHash[ls_Peptide]['spots'] = Set.new
				lk_PeptideHash[ls_Peptide]['found'] = Hash.new
				lk_PeptideHash[ls_Peptide]['proteins'] = Hash.new
			end
			lk_PeptideHash[ls_Peptide]['scans'].push(ls_Scan)
			lk_PeptideHash[ls_Peptide]['spots'].add(ls_Scan[0, ls_Scan.index('.')])
			lk_ScanHash[ls_Scan]['deflines'].each do |ls_DefLine|
				if (ls_DefLine[0, 12] == 'target_gpf__')
					lk_PeptideHash[ls_Peptide]['found']['gpf'] = true 
				else
					lk_PeptideHash[ls_Peptide]['found']['models'] = true 
					lk_PeptideHash[ls_Peptide]['proteins'][ls_DefLine] = true
				end
			end
		end
		
		
		# sort scans in lk_PeptideHash by e-value
		lk_PeptideHash.keys.each do |ls_Peptide|
			lk_PeptideHash[ls_Peptide]['scans'] = lk_PeptideHash[ls_Peptide]['scans'].sort { |a, b| lk_ScanHash[a]['e'] <=> lk_ScanHash[b]['e'] }
		end
		
		lk_GpfPeptides = Set.new
		lk_ModelPeptides = Set.new
		lk_ProteinIdentifyingModelPeptides = Set.new
		
		lk_PeptideHash.keys.each do |ls_Peptide|
			lk_GpfPeptides.add(ls_Peptide) if lk_PeptideHash[ls_Peptide]['found'].has_key?('gpf')
			lk_ModelPeptides.add(ls_Peptide) if lk_PeptideHash[ls_Peptide]['found'].has_key?('models')
			lk_ProteinIdentifyingModelPeptides.add(ls_Peptide) if lk_PeptideHash[ls_Peptide]['proteins'].size == 1
		end
		
		lk_Proteins = Hash.new
		lk_ProteinIdentifyingModelPeptides.each do |ls_Peptide|
			ls_Protein = lk_PeptideHash[ls_Peptide]['proteins'].keys.first.sub('target_', '')
			if !lk_Proteins.has_key?(ls_Protein)
				lk_Proteins[ls_Protein] = Hash.new
				lk_Proteins[ls_Protein]['spectraCount'] = 0
				lk_Proteins[ls_Protein]['peptides'] = Hash.new
			end
			lk_Proteins[ls_Protein]['spectraCount'] += lk_PeptideHash[ls_Peptide]['scans'].size
			lk_Proteins[ls_Protein]['peptides'][ls_Peptide] = lk_PeptideHash[ls_Peptide]['scans'].size
		end
		
		lk_Result = Hash.new
		
		lk_Result[:goodScans] = lk_GoodScans
		lk_Result[:scanHash] = lk_ScanHash
		lk_Result[:peptideHash] = lk_PeptideHash
		lk_Result[:gpfPeptides] = lk_GpfPeptides
		lk_Result[:modelPeptides] = lk_ModelPeptides
		lk_Result[:proteinIdentifyingModelPeptides] = lk_ProteinIdentifyingModelPeptides
		lk_Result[:proteins] = lk_Proteins
		lk_Result[:eThresholds] = lk_EThresholds
		
		return lk_Result
	end
	
	def run()
		if (@param[:mode] == 'merge' || @input[:omssaResults].size == 1)
			# merge OMSSA results
			lk_Result = evaluateFiles(@input[:omssaResults])
			
			lk_GoodScans = lk_Result[:goodScans]
			lk_ScanHash = lk_Result[:scanHash]
			lk_PeptideHash = lk_Result[:peptideHash]
			lk_GpfPeptides = lk_Result[:gpfPeptides]
			lk_ModelPeptides = lk_Result[:modelPeptides]
			lk_ProteinIdentifyingModelPeptides = lk_Result[:proteinIdentifyingModelPeptides]
			lk_Proteins = lk_Result[:proteins]
			lk_EThresholds = lk_Result[:eThresholds]
			
			lk_ProteinsBySpectraCount = lk_Proteins.keys.sort { |a, b| lk_Proteins[b]['spectraCount'] <=> lk_Proteins[a]['spectraCount']}
			
			puts "Cropped #{lk_GoodScans.size} spectra from #{lk_ScanHash.size}."
			puts "Unique peptides identified: #{lk_PeptideHash.size}."
			puts "Peptides found by both GPF and models: #{(lk_GpfPeptides & lk_ModelPeptides).size}."
			puts "Peptides found by GPF alone: #{(lk_GpfPeptides - lk_ModelPeptides).size}."
			puts "Peptides found by models alone: #{(lk_ModelPeptides - lk_GpfPeptides).size}."
			puts "Model peptides that identify a protein: #{lk_ProteinIdentifyingModelPeptides.size}"
			puts "Model peptides that appear in more than one protein: #{(lk_ModelPeptides - lk_ProteinIdentifyingModelPeptides).size}."
			puts "Proteins identified: #{lk_Proteins.size}."
			
			if @output[:identifiedProteins]
				File.open(@output[:identifiedProteins], 'w') do |lk_Out|
					lk_Out.puts('Spectra count, Protein')
					lk_ProteinsBySpectraCount.each do |ls_Protein|
						lk_Out.puts("#{lk_Proteins[ls_Protein]['spectraCount']},\"#{ls_Protein}\"")
					end
				end
			end
			
			if @output[:htmlReport]
				File.open(@output[:htmlReport], 'w') do |lk_Out|
					lk_ShortScans = Hash.new()
					lk_GoodScans.each do |ls_Scan|
						ls_ShortScan = ls_Scan.split('.').first
						lk_ShortScans[ls_ShortScan] ||= Array.new
						lk_ShortScans[ls_ShortScan].push(ls_Scan)
					end
					lk_ShortScanKeys = lk_ShortScans.keys
					lk_ShortScanKeys.sort! { |x, y| String::natcmp(x, y) }
					
					lk_Out.puts '<html>'
					lk_Out.puts '<head>'
					lk_Out.puts '<title>OMSSA Report</title>'
					lk_Out.puts '<style type=\'text/css\'>'
					lk_Out.puts 'body {font-family: Verdana; font-size: 10pt;}'
					lk_Out.puts 'h1 {font-size: 14pt;}'
					lk_Out.puts 'h2 {font-size: 12pt; border-top: 1px solid #888; border-bottom: 1px solid #888; padding-top: 0.2em; padding-bottom: 0.2em; background-color: #e8e8e8; }'
					lk_Out.puts 'h3 {font-size: 10pt; }'
					lk_Out.puts 'h4 {font-size: 10pt; font-weight: normal;}'
					lk_Out.puts 'ul {padding-left: 0;}'
					lk_Out.puts 'ol {padding-left: 0;}'
					lk_Out.puts 'li {margin-left: 2em;}'
					lk_Out.puts '.default { }'
					lk_Out.puts '.nonDefault { background-color: #ada;}'
					lk_Out.puts 'table {border-collapse: collapse;} '
					lk_Out.puts 'table tr {text-align: left; font-size: 10pt;}'
					lk_Out.puts 'table th, td {vertical-align: top; border: 1px solid #888; padding: 0.2em;}'
					lk_Out.puts 'table th {font-weight: bold;}'
					lk_Out.puts '.gpf-confirm { background-color: #aed16f; }'
					lk_Out.puts '.toggle { cursor: pointer; text-decoration: underline; color: #aaa; }'
					lk_Out.puts '.toggle:hover { color: #000; }'
					lk_Out.puts '</style>'
					lk_Out.puts "<script type='text/javascript'>"
					lk_Out.puts "/*<![CDATA[*/"
					lk_Out.puts "function toggle(as_Name, as_Display) {"
					lk_Out.puts "lk_Elements = document.getElementsByClassName(as_Name);"
					lk_Out.puts "for (var i = 0; i < lk_Elements.length; ++i)"
					lk_Out.puts "lk_Elements[i].style.display = lk_Elements[i].style.display == 'none' ? as_Display : 'none';"
					lk_Out.puts "}"
					lk_Out.puts "/*]]>*/"
					lk_Out.puts "</script>"
					lk_Out.puts '</head>'
					lk_Out.puts '<body>'
					lk_Out.puts '<h1>OMSSA Report</h1>'
					lk_Out.puts '<p>'
					lk_Out.puts "Processed #{lk_ScanHash.size} spectra in #{lk_ShortScanKeys.size} spot#{lk_ShortScanKeys.size == 1 ? '' : 's'}.<br />"
					lk_Out.puts "Significant identifications could be made in #{lk_GoodScans.size} of these spectra at a maximum false positive ratio of #{@param[:targetFpr]}%.<br />"
					lk_Out.puts '</p>'
					lk_Out.puts '<h2>Contents</h2>'
					lk_Out.puts '<ol>'
					lk_SpotLinks = Array.new
					lk_ShortScanKeys.each { |ls_Spot| lk_SpotLinks.push("<a href='#subheader-spot-#{ls_Spot}'>#{ls_Spot}</a>") }
					lk_Out.puts "<li><a href='#header-identified-proteins-by-spectra-count'>Identified proteins by spectra count</a></li>"
					lk_Out.puts "<li><a href='#header-identified-proteins-by-spot'>Identified proteins by spot</a> <span class='toggle' onclick='toggle(\"toc-proteins-by-spot-spots\", \"inline\")'>[show spots]</span><span class='toc-proteins-by-spot-spots' style='display: none;'><br />(#{lk_SpotLinks.join(', ')})</span></li>"
					lk_Out.puts "<li><a href='#header-identified-proteins-by-distinct-peptide-count'>Identified proteins by distinct peptide count</a></li>"
					lk_Out.puts "<li><a href='#header-new-gpf-peptides'>Additional peptides identified by GPF</a></li>" if (lk_GpfPeptides - lk_ModelPeptides).size > 0
					lk_Out.puts "<li><a href='#header-ambiguous-peptides'>Identified peptides that appear in more than one model protein</a></li>" if (lk_ModelPeptides - lk_ProteinIdentifyingModelPeptides).size > 0
					lk_Out.puts "<li><a href='#header-e-thresholds'>E-value thresholds by spot</a></li>"
					lk_Out.puts '</ol>'
				
					lk_Out.puts "<h2 id='header-identified-proteins-by-spectra-count'>Identified proteins by spectra count</h2>"
					lk_Out.puts "<p>This table contains all model proteins that could be identified. Peptides that have additionally been found by de novo prediction and GPF search are <span class='gpf-confirm'>highlighted</span>.</p>"
					
					lk_Out.puts '<table>'
					lk_Out.puts "<tr><th>Protein</th><th>Protein spectra count</th><th>Peptides</th><th>Peptide spectra count</th></tr>"
					lk_Out.print '<tr>'
					lb_Open0 = true
					li_ToggleCounter = 0
					lk_ProteinsBySpectraCount.each do |ls_Protein|
						lb_Open1 = true
						lk_Out.print "<tr>" unless lb_Open0
						lk_FoundInSpots = Set.new
						lk_Proteins[ls_Protein]['peptides'].keys.each do |ls_Peptide|
							lk_FoundInSpots.merge(lk_PeptideHash[ls_Peptide]['spots'])
						end
						lk_FoundInSpots = lk_FoundInSpots.to_a
						lk_FoundInSpots.sort! { |x, y| String::natcmp(x, y) }
						lk_FoundInSpots.collect! { |x| "<a href='#subheader-spot-#{x}'>#{x}</a>" }
						ls_FoundInSpots = lk_FoundInSpots.join(', ')
						li_ToggleCounter += 1
						ls_ToggleClass = "proteins-by-spectra-count-#{li_ToggleCounter}"
						lk_Out.print "<td rowspan='#{lk_Proteins[ls_Protein]['peptides'].size}'>#{ls_Protein.sub('target_', '')} <i>(<span class='toggle' onclick='toggle(\"#{ls_ToggleClass}\", \"inline\")'>found in:</span><span class='#{ls_ToggleClass}' style='display: none;'> #{ls_FoundInSpots}</span>)</i></td>"
						lk_Out.print "<td rowspan='#{lk_Proteins[ls_Protein]['peptides'].size}'>#{lk_Proteins[ls_Protein]['spectraCount']}</td>"
						lk_PeptidesSorted = lk_Proteins[ls_Protein]['peptides'].keys.sort { |x, y| lk_Proteins[ls_Protein]['peptides'][y] <=> lk_Proteins[ls_Protein]['peptides'][x]}
						lk_PeptidesSorted.each do |ls_Peptide|
							lk_Out.print "<tr>" unless lb_Open1
							ls_CellStyle = lk_PeptideHash[ls_Peptide]['found']['gpf']? ' class=\'gpf-confirm\'' : ''
							lk_Out.print "<td><span#{ls_CellStyle}>#{ls_Peptide}</span></td><td>#{lk_Proteins[ls_Protein]['peptides'][ls_Peptide]}</td></tr>\n"
							lb_Open0 = false
							lb_Open1 = false
						end
					end
					lk_Out.puts '</table>'
					
					
					lk_Out.puts "<h2 id='header-identified-proteins-by-spot'>Identified proteins by spot</h2>"
					lk_Out.puts "<p>This table contains all model proteins that could be identified, sorted by spot. Peptides that have additionally been found by de novo prediction and GPF search are <span class='gpf-confirm'>highlighted</span>.</p>"
					
					lk_Out.puts '<table>'
					lk_ShortScanKeys.each do |ls_Spot|
				#WLQYSEVIHAR:
				#  scans: [MT_HydACPAN_1_300407.100.100.2, ...]
				#  spots: (MT_HydACPAN_1_300407) (set)
				#  found: {gpf, models}
				#  proteins: {x => true, y => true}
						lk_SpotProteins = Hash.new
						lk_ProteinIdentifyingModelPeptides.each do |ls_Peptide|
							lk_Peptide = lk_PeptideHash[ls_Peptide]
							next unless lk_Peptide['spots'].include?(ls_Spot)
							li_PeptideCount = lk_Peptide['scans'].select { |x| x[0, ls_Spot.size] == ls_Spot }.size
							lk_Peptide['proteins'].keys.each do |ls_Protein|
								lk_SpotProteins[ls_Protein] ||= Hash.new
								lk_SpotProteins[ls_Protein]['peptides'] ||= Hash.new
								lk_SpotProteins[ls_Protein]['peptides'][ls_Peptide] ||= 0
								lk_SpotProteins[ls_Protein]['peptides'][ls_Peptide] += li_PeptideCount
								lk_SpotProteins[ls_Protein]['count'] ||= 0
								lk_SpotProteins[ls_Protein]['count'] += li_PeptideCount
							end
						end
						#lk_Out.puts "<h3 id='subheader-spot-#{ls_Spot}'>#{ls_Spot}</h3>"
						lk_Out.puts "<tr id='subheader-spot-#{ls_Spot}'><td style='border-style: none; background-color: #fff; padding-top: 2em; padding-bottom: 1em;' colspan='4'><span style='font-weight: bold;'>#{ls_Spot}</span></td></tr>"
						lk_Out.puts '<tr><th>Protein</th><th>Protein spectra count</th><th>Peptides</th><th>Peptide spectra count</th></tr>'
						lk_Out.print '<tr>'
						lb_Open0 = true
						lk_SpotProteinsSorted = lk_SpotProteins.keys.sort { |x, y| lk_SpotProteins[y]['count'] <=> lk_SpotProteins[x]['count'] }
						lk_SpotProteinsSorted.each do |ls_Protein|
							lb_Open1 = true
							lk_Out.print "<tr>" unless lb_Open0
							lk_Out.print "<td rowspan='#{lk_SpotProteins[ls_Protein]['peptides'].size}'>#{ls_Protein.sub('target_', '')}</td>"
							lk_Out.print "<td rowspan='#{lk_SpotProteins[ls_Protein]['peptides'].size}'>#{lk_SpotProteins[ls_Protein]['count']}</td>"
							lk_PeptidesSorted = lk_SpotProteins[ls_Protein]['peptides'].keys.sort { |x, y| lk_SpotProteins[ls_Protein]['peptides'][y] <=> lk_SpotProteins[ls_Protein]['peptides'][x]}
							lk_PeptidesSorted.each do |ls_Peptide|
								lk_Out.print "<tr>" unless lb_Open1
								ls_CellStyle = lk_PeptideHash[ls_Peptide]['found']['gpf']? ' class=\'gpf-confirm\'' : ''
								lk_Out.print "<td><span#{ls_CellStyle}>#{ls_Peptide}</span></td><td>#{lk_SpotProteins[ls_Protein]['peptides'][ls_Peptide]}</td></tr>\n"
								lb_Open0 = false
								lb_Open1 = false
							end
						end
					end
					lk_Out.puts '</table>'
					
					lk_Out.puts "<h2 id='header-identified-proteins-by-distinct-peptide-count'>Identified proteins by distinct peptide count</h2>"
					lk_Out.puts "<p>This table contains all model proteins that could be identified, sorted by the number of distinct peptides that identified the protein.</p>"
					
					lk_ProteinsByDistinctPeptideCount = lk_Proteins.keys.sort { |a, b| lk_Proteins[b]['peptides'].size <=> lk_Proteins[a]['peptides'].size}
					lk_Out.puts '<table>'
					lk_Out.puts '<tr><th>Protein</th><th>Distinct peptide count</th><th>Peptides</th></tr>'
					lk_ProteinsByDistinctPeptideCount.each do |ls_Protein|
						lk_Out.puts "<tr><td>#{ls_Protein}</td><td>#{lk_Proteins[ls_Protein]['peptides'].size}</td><td>#{lk_Proteins[ls_Protein]['peptides'].keys.sort.join(', ')}</td></tr>"
					end
					lk_Out.puts '</table>'
					
					
					if (lk_GpfPeptides - lk_ModelPeptides).size > 0
						lk_Out.puts "<h2 id='header-new-gpf-peptides'>Additional peptides identified by GPF</h2>" 
						lk_Out.puts '<p>These peptides have been significantly identified by de novo prediction and an error-tolerant GPF search, which means that these identified peptides are very probably correct although they are not part of the gene models used for the search.</p>'
						lk_GpfOnlyPeptides = (lk_GpfPeptides - lk_ModelPeptides).to_a.sort! do |x, y|
							lk_PeptideHash[x]['scans'].size == lk_PeptideHash[y]['scans'].size ? x <=> y : lk_PeptideHash[y]['scans'].size <=> lk_PeptideHash[x]['scans'].size
						end
						lk_Out.puts '<table>'
						lk_Out.puts '<tr><th>Count</th><th>Peptide</th><th>Scan</th><th>E-value</th></tr>'
						lk_GpfOnlyPeptides.each do |ls_Peptide|
							li_ScanCount = lk_PeptideHash[ls_Peptide]['scans'].size
							lk_Out.puts "<tr><td rowspan='#{li_ScanCount}'>#{li_ScanCount}</td><td rowspan='#{li_ScanCount}'>#{ls_Peptide}</td><td>#{lk_PeptideHash[ls_Peptide]['scans'].first}</td><td>#{sprintf('%e', lk_ScanHash[lk_PeptideHash[ls_Peptide]['scans'].first]['e'])}</td></tr>"
							(1...li_ScanCount).each do |i|
								lk_Out.puts "<tr><td>#{lk_PeptideHash[ls_Peptide]['scans'][i]}</td><td>#{sprintf('%e', lk_ScanHash[lk_PeptideHash[ls_Peptide]['scans'][i]]['e'])}</td></tr>"
							end
						end
						lk_Out.puts '</table>'
					end
	
					if (lk_ModelPeptides - lk_ProteinIdentifyingModelPeptides).size > 0
						lk_Out.puts "<h2 id='header-ambiguous-peptides'>Identified peptides that appear in more than one model protein</h2>"
						lk_Out.puts '<p>These peptides have been significantly identified but could not be used to identify a protein because they appear in multiple proteins. Peptides that have additionally been found by de novo prediction and GPF searching are <span class=\'gpf-confirm\'>highlighted</span>.</p>'
						lk_AmbiguousPeptides = (lk_ModelPeptides - lk_ProteinIdentifyingModelPeptides).to_a.sort! do |x, y|
							lk_PeptideHash[x]['scans'].size == lk_PeptideHash[y]['scans'].size ? x <=> y : lk_PeptideHash[y]['scans'].size <=> lk_PeptideHash[x]['scans'].size
						end
						lk_Out.puts '<table>'
						lk_Out.puts '<tr><th>Count</th><th>Peptide</th><th>Proteins</th><th>Scan</th><th>E-value</th></tr>'
						lk_AmbiguousPeptides.each do |ls_Peptide|
							li_ScanCount = lk_PeptideHash[ls_Peptide]['scans'].size
							ls_CellStyle = lk_PeptideHash[ls_Peptide]['found']['gpf']? ' class=\'gpf-confirm\'' : ''
							lk_Out.puts "<tr><td rowspan='#{li_ScanCount}'>#{li_ScanCount}</td><td rowspan='#{li_ScanCount}'><span#{ls_CellStyle}>#{ls_Peptide}</span></td><td rowspan='#{li_ScanCount}'><ul>#{lk_PeptideHash[ls_Peptide]['proteins'].keys.collect { |x| "<li>#{x}</li>" }.join(' ')}</ul></td><td>#{lk_PeptideHash[ls_Peptide]['scans'].first}</td><td>#{sprintf('%e', lk_ScanHash[lk_PeptideHash[ls_Peptide]['scans'].first]['e'])}</td></tr>"
							(1...li_ScanCount).each do |i|
								lk_Out.puts "<tr><td>#{lk_PeptideHash[ls_Peptide]['scans'][i]}</td><td>#{sprintf('%e', lk_ScanHash[lk_PeptideHash[ls_Peptide]['scans'][i]]['e'])}</td></tr>"
							end
						end
						lk_Out.puts '</table>'
					end
					
					lk_Out.puts "<h2 id='header-e-thresholds'>E-value thresholds by spot</h2>"
					lk_Out.puts "<p>In the following table you find the E-value thresholds that have been determined for each spot in order to achieve a maximum false positive ratio of #{@param[:targetFpr]}%.</p>"
					lk_Out.puts '<table>'
					lk_Out.puts '<tr><th>Spot</th><th>E-value threshold</th></tr>'
					
					lk_ShortScanKeys.each do |ls_Spot|
						lk_Out.puts "<tr><td>#{ls_Spot}</td><td>#{lk_EThresholds[ls_Spot] ? sprintf('%e', lk_EThresholds[ls_Spot]) : 'n/a'}</td></tr>"
					end
					lk_Out.puts '</table>'
					
					
					lk_Out.puts '</body>'
					lk_Out.puts '</html>'
				end
			end
			
			if (@output[:identifiedPeptides])
				File.open(@output[:identifiedPeptides], 'w') do |lk_Out|
					lk_Peptides = lk_PeptideHash.keys.sort
					lk_Peptides.each do |ls_Peptide|
						lk_Out.puts ">#{ls_Peptide}"
						lk_Out.puts ls_Peptide
					end
				end
			end
			
			if (@output[:amsFile])
				File.open(@output[:amsFile], 'w') do |lk_Out|
					lk_NeededSpots = Set.new
					lk_NeededScans = Set.new
					lk_FoundToSoftware = {'models' => 'OMSSA', 'gpf' => 'GPF-OMSSA'}
					lk_SpotToSpectraFile = Hash.new
					@input[:spectra].each { |ls_Path| lk_SpotToSpectraFile[File::basename(ls_Path).split('.').first] = ls_Path } if @input[:spectra]
					
					# see which scans have to be read from the spectrum files
					lk_PeptideHash.each do |ls_Peptide, lk_Peptide|
						lk_Peptide['scans'].each do |ls_Scan|
							lk_NeededScans.add(ls_Scan)
							ls_Spot = ls_Scan.split('.').first
							lk_NeededSpots.add(ls_Spot)
						end
					end
					
					lk_MissingSpots = Array.new
					lk_NeededSpots.each { |ls_Spot|	lk_MissingSpots.push(ls_Spot) unless lk_SpotToSpectraFile[ls_Spot] }
					lk_ScanData = Hash.new
					lk_MeasuredMasses = Hash.new
					
					unless lk_MissingSpots.empty?
						puts "ATTENTION: The measures masses and spectral data for the following spots can not be included into the 2DB upload file because the following spots have not been specified as input files: #{lk_MissingSpots.join(', ')}."
					end
					
					#fetch measures masses and spectral data
					print 'Extracting spectral data...' unless lk_NeededSpots.to_a == lk_MissingSpots
					lk_NeededSpots.each do |ls_Spot|
						ls_Filename = lk_SpotToSpectraFile[ls_Spot]
						next unless ls_Filename
						
						lk_SpectrumProc = Proc.new do |ls_Filename, ls_Contents|
							if (lk_NeededScans.include?(ls_Filename))
								ls_Contents.gsub!("\n", ';')
								ls_Contents.gsub!(' ', ',')
								lk_ScanData[ls_Filename] = ls_Contents 
							end
							lk_MeasuredMasses[ls_Filename] = ls_Contents.split("\n").first.split(' ').first.to_f - 1.007825
						end
						
						DtaIterator.new(ls_Filename, lk_SpectrumProc).run
					end
					puts 'done.' unless lk_NeededSpots.to_a == lk_MissingSpots
					
					# write AMS header
					lk_Out.puts "spectrum_id!software!charge!meas_mass!cal_mass!delta_mass!scores!sequence_in!sequence_out!left_fragment!right_fragment!left_pos!right_pos!left_rf!right_rf!tic!database!reference!spectrum!search_string!"
					
					# iterate OMSSA results
					lk_PeptideHash.each do |ls_Peptide, lk_Peptide|
						lk_Peptide['scans'].each do |ls_Scan|
							lk_Peptide['found'].keys.each do |ls_Found|
								ls_Software = lk_FoundToSoftware[ls_Found]
								lk_ScanNameParts = ls_Scan.split('.')
								li_Charge = lk_ScanNameParts[lk_ScanNameParts.size - 2].to_i
								ls_Spot = lk_ScanNameParts.first
								ls_SpotFilename = lk_SpotToSpectraFile[ls_Spot]
								lf_CalculatedMass = lk_ScanHash[ls_Scan]['peptides'][ls_Peptide].to_f
								lf_MeasuredMass = lk_MeasuredMasses[ls_Scan]
								lf_MeasuredMass = lf_CalculatedMass unless lf_MeasuredMass
								lf_EValue = lk_ScanHash[ls_Scan]['e']
								ls_SpectrumData = lk_ScanData[ls_Scan]
								ls_SpectrumData = '' unless ls_SpectrumData
								# database and reference intentionally left blank because 2DB does
								# the search by itself.
								ls_Database = ''
								ls_Reference = ''
								lk_Out.puts "#{ls_Scan}!#{ls_Software}!#{li_Charge}!#{lf_MeasuredMass}!#{lf_CalculatedMass}!#{lf_MeasuredMass - lf_CalculatedMass}!fpr:#{@param[:targetFpr] / 100.0},evalue:#{lf_EValue}!!#{ls_Peptide}!!!!!!!!#{ls_Database}!#{ls_Reference}!#{ls_SpectrumData}!!"
							end
						end
					end
				end
			end
		else
			lk_RunResults = Hash.new
			# compare multiple OMSSA results
			@input[:omssaResults].each do |ls_OmssaResultFile|
				ls_Key = File::basename(ls_OmssaResultFile)
				ls_Key.sub!('omssa-results.csv', '')
				ls_Key = ls_Key[0, ls_Key.size - 1] while ((!ls_Key.empty?) && ('_-. '.include?(ls_Key[ls_Key.size - 1, 1])))
				lk_Result = evaluateFiles([ls_OmssaResultFile])
				lk_RunResults[ls_Key] = lk_Result
			end
			
			if @output[:htmlReport]
				File.open(@output[:htmlReport], 'w') do |lk_Out|
					lk_RunKeys = lk_RunResults.keys.sort { |x, y| String::natcmp(x, y) }
					
					lk_Out.puts '<html>'
					lk_Out.puts '<head>'
					lk_Out.puts '<title>OMSSA Comparison Report</title>'
					lk_Out.puts '<style type=\'text/css\'>'
					lk_Out.puts 'body {font-family: Verdana; font-size: 10pt;}'
					lk_Out.puts 'h1 {font-size: 14pt;}'
					lk_Out.puts 'h2 {font-size: 12pt; border-top: 1px solid #888; border-bottom: 1px solid #888; padding-top: 0.2em; padding-bottom: 0.2em; background-color: #e8e8e8; }'
					lk_Out.puts 'h3 {font-size: 10pt; }'
					lk_Out.puts 'h4 {font-size: 10pt; font-weight: normal;}'
					lk_Out.puts 'ul {padding-left: 0;}'
					lk_Out.puts 'ol {padding-left: 0;}'
					lk_Out.puts 'li {margin-left: 2em;}'
					lk_Out.puts '.default { }'
					lk_Out.puts '.nonDefault { background-color: #ada;}'
					lk_Out.puts 'table {border-collapse: collapse;} '
					lk_Out.puts 'table tr {text-align: left; font-size: 10pt;}'
					lk_Out.puts 'table th, td {vertical-align: top; border: 1px solid #888; padding: 0.2em;}'
					lk_Out.puts 'table th {font-weight: bold;}'
					lk_Out.puts '.gpf-confirm { background-color: #aed16f; }'
					lk_Out.puts '.toggle { cursor: pointer; text-decoration: underline; color: #aaa; }'
					lk_Out.puts '.toggle:hover { color: #000; }'
					lk_Out.puts '</style>'
					lk_Out.puts "<script type='text/javascript'>"
					lk_Out.puts "/*<![CDATA[*/"
					lk_Out.puts "function toggle(as_Name, as_Display) {"
					lk_Out.puts "lk_Elements = document.getElementsByClassName(as_Name);"
					lk_Out.puts "for (var i = 0; i < lk_Elements.length; ++i)"
					lk_Out.puts "lk_Elements[i].style.display = lk_Elements[i].style.display == 'none' ? as_Display : 'none';"
					lk_Out.puts "}"
					lk_Out.puts "/*]]>*/"
					lk_Out.puts "</script>"
					lk_Out.puts '</head>'
					lk_Out.puts '<body>'
					lk_Out.puts '<h1>OMSSA Comparison Report</h1>'
				
					lk_Out.puts '<table>'
					lk_Out.puts "<tr><th rowspan='2'>Protein</th><th colspan='#{lk_RunKeys.size}'>Spectra count</th><th rowspan='2'>std. dev.</th><th colspan='#{lk_RunKeys.size}'>Distinct peptide count</th><th rowspan='2'>std. dev.</th></tr>"
					lk_Out.puts "<tr>#{lk_RunKeys.collect { |x| '<th>' + x + '</th>'}.join('') }#{lk_RunKeys.collect { |x| '<th>' + x + '</th>'}.join('') }</tr>"
					# collect proteins from all runs
					lk_AllProteinsSet = Set.new
					lk_RunKeys.each { |ls_Key| lk_AllProteinsSet.merge(lk_RunResults[ls_Key][:proteins].keys) }
					lk_Proteins = lk_AllProteinsSet.to_a.sort { |x, y| String::natcmp(x, y) }
					lk_Proteins.each do |ls_Protein|
						lk_Out.print "<tr><td>#{ls_Protein}</td>"
						
						lk_Values = Array.new
						ls_SpectraCountString = lk_RunKeys.collect do |ls_Key|
							li_Count = 0
							li_Count = lk_RunResults[ls_Key][:proteins][ls_Protein]['spectraCount'] if lk_RunResults[ls_Key][:proteins].has_key?(ls_Protein)
							lk_Values.push(li_Count)
							"<td>#{li_Count}</td>"
						end.join('')
						lk_Out.print ls_SpectraCountString
						lk_Out.print "<td>#{sprintf("%1.2f", stddev(lk_Values))}</td>"
						
						lk_Values = Array.new
						ls_DistinctPeptidesCountString = lk_RunKeys.collect do |ls_Key|
							li_Count = 0
							li_Count = lk_RunResults[ls_Key][:proteins][ls_Protein]['peptides'].size if lk_RunResults[ls_Key][:proteins].has_key?(ls_Protein)
							lk_Values.push(li_Count)
							"<td>#{li_Count}</td>"
						end.join('')
						lk_Out.print ls_DistinctPeptidesCountString
						lk_Out.print "<td>#{sprintf("%1.2f", stddev(lk_Values))}</td>"
						
						lk_Out.print "</tr>"
						lk_Out.puts
					end
					lk_Out.puts '</table>'
					
					
					lk_Out.puts '</body>'
					lk_Out.puts '</html>'
				end
			end
		end
	end
end

lk_Object = EvaluateOmssa.new
