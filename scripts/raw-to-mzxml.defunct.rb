require 'include/proteomatic'
require 'fileutils'

class Raw2MzXML < ProteomaticScript
	def run()
		@output.each do |ls_InPath, ls_OutPath|
			puts "#{File.basename(ls_InPath)}"
			ls_TempOutPath = tempFilename('raw-to-mzxml')
			
			# call ReAdW
			lk_Arguments = Array.new
			#lk_Arguments.push('-z') if (@param[:useCompression])
			ls_Command = "#{ExternalTools::binaryPath('readw.readw')} --mzXML #{@mk_Parameters.commandLineFor('readw.readw')} #{lk_Arguments.join(' ')} \"#{ls_InPath}\" \"#{ls_TempOutPath}\""
			unless system(ls_Command)
				puts 'There was an error while executing readw.'
				exit 1
			end
			
			# strip MS 1 scans if desired
			if @param[:stripMs1Scans]
				# zip mzXML file
				ls_Command = "#{ExternalTools::binaryPath('7zip.7zip')} a #{ls_OutPath}.zip #{ls_TempOutPath}"
				system(ls_Command)
				FileUtils::rm_f(ls_TempOutPath)
			else
				# rename temp out file
				FileUtils::mv(ls_TempOutPath, ls_OutPath)
			end
			
		end
	end
end

lk_Object = Raw2MzXML.new
