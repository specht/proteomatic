require 'yaml'

lk_Occurences = Hash.new
li_Length = 1

lk_Scaffolds = Dir['/home/michael/Augustus/flat/*']
lk_Scaffolds.each do |ls_Scaffold|
	puts ls_Scaffold
	ls_Dna = File::read(ls_Scaffold)
	(0...ls_Dna.size - (li_Length - 1)).each do |i|
		ls_Token = ls_Dna[i, li_Length]
		lk_Occurences[ls_Token] ||= 0
		lk_Occurences[ls_Token] += 1
	end
end

puts lk_Occurences.to_yaml
