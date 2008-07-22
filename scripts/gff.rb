require 'yaml'

def handleFile(ak_Files, ak_Out = $stdout)
	lk_Peptides = Hash.new
	# merge all results, overwrite duplicate keys (values should be the same anyway)
	ak_Files.each { |ls_Path| lk_Peptides.merge!(YAML::load_file(ls_Path)) }
		
	lk_Peptides.each do |ls_Peptide, lk_Hits|
		li_AssemblyCount = lk_Hits.size
		next if li_AssemblyCount == 0
		lk_Hits.each_index do |li_AssemblyIndex|
			lk_Assembly = lk_Hits[li_AssemblyIndex]
			
			# adjust positions if reverse (from GPF to GFF)
			if lk_Assembly['details']['forward']
				lk_Assembly['details']['parts'].each { |lk_Part| lk_Part['position'] += 1 } 
			else
				lk_Assembly['details']['parts'].each { |lk_Part| lk_Part['position'] = lk_Part['position'] - lk_Part['length'] + 2 } 
			end
			
			lk_Hits[li_AssemblyIndex] = lk_Assembly
		end
		
		lk_NrHits = lk_Hits.collect { |x| {'parts' => x['details']['parts'], 'peptide' => x['peptide'], 'forward' => x['details']['forward']} }
		lk_NrHits = lk_NrHits.collect { |x| x.to_yaml }.uniq.collect { |x| YAML::load(x) }
		li_AssemblyCount = lk_NrHits.size
		
		# TODO: - assemblies wrong, don't validate
		#       - intron wrong
		#       - chuck out short CDS and adjacent introns
		lk_AllSpans = Array.new
		lk_NrHits.each do |lk_Assembly|
			lk_Spans = Array.new
			li_NucleotideCount = 0
			lk_Assembly['parts'].each do |lk_Part|
				li_Frame = (3 - (li_NucleotideCount % 3)) % 3
				li_Start = lk_Part['position']
				li_End = lk_Part['position'] + lk_Part['length'] - 1
				li_NucleotideCount += li_End - li_Start + 1
				lk_Spans.push({
					:start => li_Start,
					:end => li_End,
					:frame => li_Frame,
					:length => lk_Part['length'],
					:peptide => lk_Assembly['peptide'],
					:forward => lk_Assembly['forward'],
					:scaffold => lk_Assembly['parts'].first['scaffold']
				})
			end
			# chuck out all CDS that are shorter than 4 nucleotides
			lk_Spans.reject! { |lk_Span| lk_Span[:length] < 4 }
			lk_AllSpans.push(lk_Spans)
		end
		
		# chuck out duplicate span lists
		li_AssemblyId = 0
		lk_AllSpans = lk_AllSpans.collect { |x| x.to_yaml }.uniq.collect{ |x| YAML::load(x) }
		lk_AllSpans.each do |lk_Spans|
			li_AssemblyId += 1
			li_Min = nil
			li_Max = nil
			lk_Spans.each do |lk_Span|
				li_Min = lk_Span[:start] unless li_Min
				li_Max = lk_Span[:start] unless li_Max
				li_Min = lk_Span[:start] if lk_Span[:start] < li_Min
				li_Min = lk_Span[:end] if lk_Span[:end] < li_Min
				li_Max = lk_Span[:start] if lk_Span[:start] > li_Max
				li_Max = lk_Span[:end] if lk_Span[:end] > li_Max
			end
			
			ls_Scaffold = lk_Spans.first[:scaffold]
			ak_Out.puts "#{ls_Scaffold}\tGPF\tassembly\t#{li_Min}\t#{li_Max}\t1.0\t#{lk_Spans.first[:forward] ? '+' : '-'}\t0\tpept=#{lk_Spans.first[:peptide]};mult=#{lk_AllSpans.size};grp=#{lk_Spans.first[:peptide]}-#{li_AssemblyId};"
			(0...lk_Spans.size).each do |li_Index|
				lk_Span = lk_Spans[li_Index]
				# cds
				ak_Out.puts "#{ls_Scaffold}\tGPF\tCDS\t#{lk_Span[:start]}\t#{lk_Span[:end]}\t1.0\t#{lk_Spans.first[:forward] ? '+' : '-'}\t#{lk_Span[:frame]}\tpept=#{lk_Spans.first[:peptide]};mult=#{lk_AllSpans.size};grp=#{lk_Spans.first[:peptide]}-#{li_AssemblyId};"
				if (lk_Span != lk_Spans.last)
					# intron
					if (lk_Span[:forward])
						ak_Out.puts "#{ls_Scaffold}\tGPF\tintron\t#{lk_Span[:end] + 1}\t#{lk_Spans[li_Index + 1][:start] - 1}\t1.0\t#{lk_Span[:forward] ? '+' : '-'}\t.\tpept=#{lk_Span[:peptide]};mult=#{lk_AllSpans.size};grp=#{lk_Span[:peptide]}-#{li_AssemblyId};"
					else
						ak_Out.puts "#{ls_Scaffold}\tGPF\tintron\t#{lk_Spans[li_Index + 1][:end] + 1}\t#{lk_Span[:start] - 1}\t1.0\t#{lk_Span[:forward] ? '+' : '-'}\t.\tpept=#{lk_Span[:peptide]};mult=#{lk_AllSpans.size};grp=#{lk_Span[:peptide]}-#{li_AssemblyId};"
					end
				end
			end
		end
	end
end

File::open('/home/michael/Augustus/gpf-alignments.gff', 'w') { |f| handleFile(['/home/michael/Augustus/omssa/MT_HydACPAN-augustus-peptides.yaml', 
'/home/michael/Augustus/omssa/MT_HydACPAR-augustus-peptides.yaml'], f) }

