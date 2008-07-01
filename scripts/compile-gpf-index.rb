require 'include/proteomatic'
require 'include/externaltools'
require 'include/formats'
require 'net/http'
require 'net/ftp'
require 'yaml'
require 'set'


class CompileGpfIndex < ProteomaticScript
	def run()
		@output.each do |ls_InPath, ls_OutPath|
			ls_Command = "#{ExternalTools::binaryPath('gpf.gpfindex')} #{ls_InPath} #{ls_OutPath} \"\""
			unless system(ls_Command)
				puts 'There was an error while executing gpfindex.'
				exit 1
			end
		end
		
	end
end

lk_Object = CompileGpfIndex.new
