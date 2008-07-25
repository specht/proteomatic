require 'include/proteomatic'


class TransposeDna < ProteomaticScript
	def run()
		lk_Nucleotides = {'A' => 'T', 'C' => 'G', 'G' => 'C', 'T' => 'A'}
		ls_Source = @param[:nucleotides].reverse.upcase
		ls_Result = ''
		(0...ls_Source.size).each do |i|
			ls_Result += lk_Nucleotides[ls_Source[i, 1]]
		end
		puts ls_Result
	end
end

lk_Object = TransposeDna.new
