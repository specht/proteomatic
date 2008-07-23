require 'include/proteomatic'

class Raw2MzXML < ProteomaticScript
	def run()
		@output.each do |ls_InPath, ls_OutPath|
			puts "#{File.basename(ls_InPath)}"
			ls_Command = "#{ExternalTools::binaryPath('readw.readw')} --mzXML #{@mk_Parameters.commandLineFor('readw.readw')} \"#{ls_InPath}\" \"#{ls_OutPath}\""
			unless system(ls_Command)
				puts 'There was an error while executing readw.'
				exit 1
			end
		end
	end
end

lk_Object = Raw2MzXML.new
