require 'include/proteomatic'
require 'fileutils'

class Raw2MzXML < ProteomaticScript
	def run()
		@output.each do |ls_InPath, ls_OutPath|
			puts "#{File.basename(ls_InPath)}"
			ls_TempOutPath = tempFilename('raw-to-mzxml')
			
			# call ReAdW
			lk_Arguments = Array.new
			lk_Arguments.push('-z') if (@param[:useCompression])
			ls_Command = "#{ExternalTools::binaryPath('readw.readw')} --mzXML #{@mk_Parameters.commandLineFor('readw.readw')} #{lk_Arguments.join(' ')} \"#{ls_InPath}\" \"#{ls_TempOutPath}\""
			unless system(ls_Command)
				puts 'There was an error while executing readw.'
				exit 1
			end
			
			# strip MS 1 scans if desired
			if @param[:stripMs1Scans]
				print 'Stripping MS 1 scans...'
				
				lb_InMs1Scan = false
				li_CurrentLevel = 0
				
				# TODO: do this right. this is very wonky and it is unsure
				# whether this will work with all valid mzXML files.
				# for the time being and with ReAdW output, however, it does.
				File::open(ls_OutPath, 'w') do |lk_Out|
					File::open(ls_TempOutPath, 'r') do |lk_File|
						lk_File.each do |ls_Line|
							ls_Stripped = ls_Line.strip
							if (ls_Stripped[0, 7] == 'msLevel')
								ls_LocalLine = ls_Stripped.dup
								ls_LocalLine.sub!('msLevel="', '')
								ls_LocalLine.sub!('"', '')
								li_CurrentLevel = ls_LocalLine.to_i
							end
							
							lb_InMs1Scan = true if (ls_Stripped[0, 6] == '<peaks') && li_CurrentLevel == 1
							lk_Out.puts ls_Line unless lb_InMs1Scan
							lb_InMs1Scan = false if (ls_Stripped[ls_Stripped.size - 8, 8] == '</peaks>')
						end
					end
				end
				
				puts 'done.'
			else
				# rename temp out file
				FileUtils::mv(ls_TempOutPath, ls_OutPath)
			end
			
		end
	end
end

lk_Object = Raw2MzXML.new
