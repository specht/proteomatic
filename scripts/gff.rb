require 'yaml'

def handleFile(ak_Files, ak_Out = $stdout)
	lk_Peptides = Hash.new
	ak_Files.each do |ls_Path|
		lk_Peptides.merge!(YAML::load_file(ls_Path))
	end
		
	lk_Peptides.each do |ls_Peptide, lk_Hits|
		li_AssemblyCount = lk_Hits.size
		next if li_AssemblyCount == 0
		lk_Frames = Hash.new
		lk_Hits.each_index do |li_AssemblyIndex|
			lk_Assembly = lk_Hits[li_AssemblyIndex]
			
			unless lk_Assembly['details']['forward']
				#lk_Assembly['details']['parts'].reverse! 
				lk_Assembly['details']['parts'].each do |lk_Part|
					lk_Part['position'] -= lk_Part['length'] - 2
				end
			end
			# chuck out all CDS that are shorter than 4 nucleotides
			# ATTENTION: this only works as long as we allow at most one intron!
			
			# calculate frames
			li_NucleotideCount = 0
			lk_Assembly['details']['parts'].each do |lk_Part|
				lk_Frames["#{lk_Part['scaffold']}-#{lk_Part['position'] + 1}"] = (3 - (li_NucleotideCount % 3)) % 3
				lk_Frames["#{lk_Part['scaffold']}-#{lk_Part['position'] + lk_Part['length']}"] = (3 - (li_NucleotideCount % 3)) % 3
				li_NucleotideCount += lk_Part['length']
			end
			
			lk_Assembly['details']['parts'] = lk_Assembly['details']['parts'].select { |x| x['length'] >= 4 }
			lk_Assembly['gffFrames'] = lk_Frames
			
			lk_Hits[li_AssemblyIndex] = lk_Assembly
		end
		
		lk_NrHits = lk_Hits.collect { |x| {'parts' => x['details']['parts'], 'peptide' => x['peptide'], 'forward' => x['details']['forward']} }
		lk_NrHits = lk_NrHits.collect { |x| x.to_yaml }.uniq.collect { |x| YAML::load(x) }
		li_AssemblyCount = lk_NrHits.size
		
		li_AssemblyId = 0
		lk_Lines = Array.new
		lk_NrHits.each do |lk_Assembly|
			li_AssemblyId += 1
			lk_Positions = Array.new
			lk_Assembly['parts'].each do |lk_Part|
				lk_Positions.push(lk_Part['position']) unless (lk_Part == lk_Assembly['parts'].first)
				lk_Positions.push(lk_Part['position'] + 1)
				lk_Positions.push(lk_Part['position'] + lk_Part['length'])
				lk_Positions.push(lk_Part['position'] + lk_Part['length'] + 1) unless (lk_Part == lk_Assembly['parts'].last)
			end
			
			lk_Positions.reverse! unless lk_Assembly['forward']
			ls_Scaffold = lk_Assembly['parts'].first['scaffold']
			lk_Lines.push("#{ls_Scaffold}\tGPF\tassembly\t#{lk_Positions.first}\t#{lk_Positions.last}\t1.0\t#{lk_Assembly['forward'] ? '+' : '-'}\t0\tpept=#{lk_Assembly['peptide']};mult=#{li_AssemblyCount};grp=#{lk_Assembly['peptide']}-#{li_AssemblyId};")
			li_PositionIndex = 0
			while (li_PositionIndex < lk_Positions.size - 1)
				li_Start = lk_Positions[li_PositionIndex]
				li_End = lk_Positions[li_PositionIndex + 1]
				unless (lk_Assembly['forward'])
					li_Temp = li_Start
					li_Start = li_End
					li_End = li_Temp
				end
				if (li_PositionIndex % 4 == 0)
					# cds
					li_Frame = lk_Frames["#{ls_Scaffold}-#{li_Start}"]
					lk_Lines.push("#{ls_Scaffold}\tGPF\tCDS\t#{li_Start}\t#{li_End}\t1.0\t#{lk_Assembly['forward'] ? '+' : '-'}\t#{li_Frame}\tpept=#{lk_Assembly['peptide']};mult=#{li_AssemblyCount};grp=#{lk_Assembly['peptide']}-#{li_AssemblyId};")
				else
					# intron
					lk_Lines.push("#{ls_Scaffold}\tGPF\tintron\t#{li_Start}\t#{li_End}\t1.0\t#{lk_Assembly['forward'] ? '+' : '-'}\t.\tpept=#{lk_Assembly['peptide']};mult=#{li_AssemblyCount};grp=#{lk_Assembly['peptide']}-#{li_AssemblyId};")
				end
				li_PositionIndex += 2
			end
		end
		ak_Out.puts lk_Lines.join("\n")
	end
end

#handleFile(['/home/michael/Augustus/omssa/MT_HydACPAN-augustus-gpf-results.yaml', '/home/michael/Augustus/omssa/MT_HydACPAR-augustus-gpf-results.yaml'])

File::open('/home/michael/augustus/gpf-alignments.gff', 'w') { |f| handleFile(['/home/michael/augustus/omssa/MT_HydACPAN-augustus-peptides.yaml', 
'/home/michael/augustus/omssa/MT_HydACPAR-augustus-peptides.yaml'], f) }

