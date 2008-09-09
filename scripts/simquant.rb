require 'include/proteomatic'
require 'include/externaltools'
require 'include/fasta'
require 'include/spectrum'
require 'yaml'
require 'fileutils'

class SimQuant < ProteomaticScript
	def run()
		if @param[:peptides].empty?
			puts 'Error: no peptides have been specified.'
			exit 1
		end
		lk_Peptides = @param[:peptides].split(%r{[,;\s/]+})
		lk_Peptides.reject! { |x| x.strip.empty? }
		puts "Searching for #{lk_Peptides.size} peptides..."
		@input[:spectra].each do |ls_SpectraPath|
			ls_Command = "#{ExternalTools::binaryPath('simquant.simquant')} #{ls_SpectraPath} #{lk_Peptides.join(' ')}"
			puts 'There was an error while executing simquant.' unless system(ls_Command)
		end
	end
end

lk_Object = SimQuant.new
