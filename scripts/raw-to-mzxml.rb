require 'include/proteomatic'

class Raw2MzXML < ProteomaticScript
	def run()
		@output.each do |ls_InPath, lk_OutPath|
			puts "#{File.basename(ls_Path)}"
			ls_Command = "#{ExternalTools::binaryPath('readw.readw')} \"#{ls_InPath}\" p \"#{ls_OutPath}\""
			unless system(ls_Command)
				puts 'There was an error while executing readw.'
				exit 1
			end
		end
	end
end

lk_Object = Raw2MzXML.new
