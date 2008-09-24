require 'include/proteomatic'
require 'fileutils'

class Raw2MzXML < ProteomaticScript
	def run()
		@output.each do |ls_InPath, ls_OutPath|
			puts "#{File.basename(ls_InPath)}"
			ls_TempOutPath = tempFilename('raw-to-mzxml')
			FileUtils.mkpath(ls_TempOutPath)
			
			# call ReAdW
			lk_Arguments = Array.new
			ls_Command = "#{ExternalTools::binaryPath('readw.readw')} --mzXML #{@mk_Parameters.commandLineFor('readw.readw')} #{lk_Arguments.join(' ')} \"#{ls_InPath}\" \"#{File::join(ls_TempOutPath, File::basename(ls_OutPath).sub('.zip.proteomatic.part', ''))}\""
			unless system(ls_Command)
				puts 'Error: There was an error while executing readw.'
				exit 1
			end

			ls_OldDir = Dir::pwd()
			
			ls_7ZipPath = File::join(Dir.pwd(), ExternalTools::binaryPath('7zip.7zip'))
			ls_TempPath = File::join(File::dirname(ls_OutPath), ls_TempOutPath)
			Dir.chdir(ls_TempPath)
			
			# zip mzXML file
			ls_Command = "#{ls_7ZipPath} a -tzip #{File::basename(ls_OutPath)} #{File::basename(ls_OutPath).sub('.zip.proteomatic.part', '')} -mx5"
			puts ls_Command
			unless system(ls_Command)
				puts 'Error: There was an error while executing 7zip.'
				exit 1
			end
			FileUtils::mv(File::basename(ls_OutPath), File::join('..', File::basename(ls_OutPath).sub('.proteomatic.part', '')))

			Dir.chdir(ls_OldDir)
			FileUtils::rm_f(ls_TempOutPath)
		end
	end
end

lk_Object = Raw2MzXML.new
