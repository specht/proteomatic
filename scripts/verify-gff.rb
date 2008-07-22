require 'yaml'

lk_Lines = File::read('/home/michael/Augustus/gpf-alignments.gff')
li_Nucleotides = 0
ls_CurrentGroup = ''
lk_Lines.each do |ls_Line|
	ls_Line.strip!
	lk_Line = ls_Line.split("\t")
	if (lk_Line[3].to_i > lk_Line[4].to_i)
		puts "Error in #{ls_Line}."
	end
	if (lk_Line[4].to_i - lk_Line[3].to_i + 1 < 4)
		puts "Error in #{ls_Line}."
	end
	ls_Group = lk_Line[8].split(';')[2].sub('grp=', '')
	if (ls_Group != ls_CurrentGroup)
		li_Nucleotides = 0
		ls_CurrentGroup = ls_Group
	end
	li_Frame = lk_Line[7].to_i
	if (lk_Line[2] == 'CDS')
#		if (li_Frame != (3 - (li_Nucleotides % 3)) % 3)
#			puts "Frame error in #{ls_Line}."
#		end
		li_Nucleotides += lk_Line[4].to_i - lk_Line[3].to_i + 1
	end
	#puts lk_Line.to_yaml
	#exit
end
