require 'include/proteomatic'
require 'include/fastercsv'
require 'include/misc'
require 'set'
require 'yaml'

class EvaluateOmssa < ProteomaticScript
	def run()
		lk_CsvFiles = @input[:omssaResults]
		
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
		
		lk_CsvFiles.each do |ls_Filename|
			ls_Spot = File::basename(ls_Filename).split('.').first
			lk_File = File.open(ls_Filename, 'r')
			
			lk_ScanHash[ls_Spot] = Hash.new
			
			# skip header
			lk_File.readline
			
			lk_File.each do |ls_Line|
				lk_Line = ls_Line.parse_csv()
				ls_Scan = lk_Line[1]
				ls_Peptide = lk_Line[2]
				lf_E = lk_Line[3]
				ls_DefLine = lk_Line[9]
				
				lk_ScanHash[ls_Spot][ls_Scan] = Hash.new if !lk_ScanHash[ls_Spot].has_key?(ls_Scan)
				if (!lk_ScanHash[ls_Spot][ls_Scan].has_key?('e') || lf_E < lk_ScanHash[ls_Spot][ls_Scan]['e'])
					# clear scan hash
					lk_ScanHash[ls_Spot][ls_Scan]['peptides'] = {ls_Peptide => true}
					lk_ScanHash[ls_Spot][ls_Scan]['deflines'] = [ls_DefLine]
					lk_ScanHash[ls_Spot][ls_Scan]['e'] = lf_E;
				elsif (lf_E == lk_ScanHash[ls_Spot][ls_Scan]['e'])
					lk_ScanHash[ls_Spot][ls_Scan]['peptides'][ls_Peptide] = true
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
		
		puts "Cropped #{lk_GoodScans.size} spectra from #{lk_ScanHash.size}."
		puts "Maximum false positive rate is #{param('targetFpr')}%."
		
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
		
		puts "Unique peptides identified: #{lk_PeptideHash.size}."
		
		lk_GpfPeptides = Set.new
		lk_ModelPeptides = Set.new
		lk_ProteinIdentifyingModelPeptides = Set.new
		
		lk_PeptideHash.keys.each do |ls_Peptide|
			lk_GpfPeptides.add(ls_Peptide) if lk_PeptideHash[ls_Peptide]['found'].has_key?('gpf')
			lk_ModelPeptides.add(ls_Peptide) if lk_PeptideHash[ls_Peptide]['found'].has_key?('models')
			lk_ProteinIdentifyingModelPeptides.add(ls_Peptide) if lk_PeptideHash[ls_Peptide]['proteins'].size == 1
		end
		
		puts "Peptides found by both GPF and models: #{(lk_GpfPeptides & lk_ModelPeptides).size}."
		puts "Peptides found by GPF alone: #{(lk_GpfPeptides - lk_ModelPeptides).size}."
		puts "Peptides found by models alone: #{(lk_ModelPeptides - lk_GpfPeptides).size}."
		puts "Model peptides that identify a protein: #{lk_ProteinIdentifyingModelPeptides.size}"
		puts "Model peptides that appear in more than one protein: #{(lk_ModelPeptides - lk_ProteinIdentifyingModelPeptides).size}."
		
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
		
		lk_ProteinsBySpectraCount = lk_Proteins.keys.sort { |a, b| lk_Proteins[b]['spectraCount'] <=> lk_Proteins[a]['spectraCount']}
		
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
				lk_Out.puts '</style>'
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
				lk_Out.puts "<li><a href='#header-identified-proteins-by-spot'>Identified proteins by spot</a><br />(#{lk_SpotLinks.join(', ')})</li>"
				lk_Out.puts "<li><a href='#header-new-gpf-peptides'>Additional peptides identified by GPF</a></li>" if (lk_GpfPeptides - lk_ModelPeptides).size > 0
				lk_Out.puts "<li><a href='#header-ambiguous-peptides'>Identified peptides that appear in more than one model protein</a></li>" if (lk_ModelPeptides - lk_ProteinIdentifyingModelPeptides).size > 0
				lk_Out.puts "<li><a href='#header-e-thresholds'>E-value thresholds by spot</a></li>"
				lk_Out.puts '</ol>'
			
				lk_Out.puts "<h2 id='header-identified-proteins-by-spot'>Identified proteins by spot</h2>"
				lk_Out.puts "<p>This table contains all model proteins that could be identified. Peptides that have additionally been found by de novo prediction and GPF search are <span class='gpf-confirm'>highlighted</span>.</p>"
				
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
					lk_Out.puts "<tr><td>#{ls_Spot}</td><td>#{sprintf('%e', lk_EThresholds[ls_Spot])}</td></tr>"
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
=begin
		if writeOutFile?('gpfOnlyPeptides')
			lk_GpfPeptides = (lk_GpfPeptides - lk_ModelPeptides).to_a
			lk_GpfPeptides.sort! { |a, b| lk_ScanHash[lk_PeptideHash[a]['scans'].first]['e'] <=> lk_ScanHash[lk_PeptideHash[b]['scans'].first]['e']}
		
			lk_Out = File.open(outFilename('gpfOnlyPeptides'), 'w')
			lk_Out.puts('E-value, Peptide')
			lk_GpfPeptides.each { |ls_Peptide| lk_Out.puts "#{lk_ScanHash[lk_PeptideHash[ls_Peptide]['scans'].first]['e']},#{ls_Peptide}" }
			lk_Out.close()
		end
		
		if writeOutFile?('proteinDetails')
			lk_Out = File.open(outFilename('proteinDetails'), 'w')
			lk_ProteinsBySpectraCount.each do |ls_Protein|
				lk_Protein = lk_Proteins[ls_Protein]
				lk_Out.puts("#{ls_Protein}:")
				lk_Protein['peptides'].keys.each do |ls_Peptide|
					lk_Out.puts("  - #{ls_Peptide}")
					lk_Scans = Hash.new
					
					lk_PeptideHash[ls_Peptide]['scans'].each do |ls_Scan|
						ls_ShortScan = ls_Scan[0, ls_Scan.index('.')]
						lk_Scans[ls_ShortScan] = 0 if !lk_Scans.has_key?(ls_ShortScan)
						lk_Scans[ls_ShortScan] += 1
					end
					lk_Scans.keys.each do |ls_Scan|
						lk_Out.puts "      - #{ls_Scan}: #{lk_Scans[ls_Scan]}"
					end
				end
			end
			lk_Out.close
		end
=end
	end
end

lk_Object = EvaluateOmssa.new
