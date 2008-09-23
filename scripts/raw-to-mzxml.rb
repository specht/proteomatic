# Copyright (c) 2007-2008 Michael Specht
# 
# This file is part of Proteomatic.
# 
# Proteomatic is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Proteomatic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Proteomatic.  If not, see <http://www.gnu.org/licenses/>.

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
