require 'include/proteomatic'
require 'include/externaltools'
require 'include/fasta'
require 'include/formats'
require 'include/spectrum'
require 'yaml'
require 'fileutils'

class SimQuant < ProteomaticScript
	def run()
		lk_Peptides = @param[:peptides].split(%r{[,;\s/]+})
		lk_Peptides.reject! { |x| x.strip.empty? }
		
		@input[:peptidesFile].each do |ls_Path|
			next unless fileMatchesFormat(ls_Path, 'txt')
			ls_File = File::read(ls_Path)
			ls_File.each do |ls_Line|
				ls_Peptide = ls_Line.strip
				next if ls_Peptide.empty?
				lk_Peptides.push(ls_Peptide)
			end
		end
		
		lk_Peptides.uniq!
		if lk_Peptides.empty?
			puts 'Error: no peptides have been specified.'
			exit 1
		end
		
		puts "Searching for #{lk_Peptides.size} peptides..."
		@input[:spectra].each do |ls_SpectraPath|
			ls_Command = "#{ExternalTools::binaryPath('simquant.simquant')} -ox #{@output[:xhtmlReport]} #{ls_SpectraPath} #{lk_Peptides.join(' ')}"
			puts 'There was an error while executing simquant.' unless system(ls_Command)
		end
	end
end

lk_Object = SimQuant.new
