require 'include/proteomatic'

class MergeFasta < ProteomaticScript
	def run()
		lk_Databases = @input[:databases]
		
		# merge all databases
		
		if @output[:merged]
			puts "Merging #{lk_Databases.size} fasta databases to #{outFilename('merged')}..."
		
			File.open(@output[:merged], 'w') do |lk_Out|
				lk_Databases.each do |ls_Filename|
					ls_Basename = File.basename(ls_Filename)
					File.open(ls_Filename, 'r') do |lk_File|
						lk_File.each do |ls_Line|
							ls_Line.strip!
							if !ls_Line.empty?
								ls_Line.insert(1, ls_Basename + ';') if (ls_Line[0, 1] == '>')
								lk_Out.puts ls_Line
							end
						end
					end
				end
			end
		end
	end
end

lk_Object = MergeFasta.new
