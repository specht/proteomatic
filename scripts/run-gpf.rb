require 'include/proteomatic'
require 'include/externaltools'
require 'include/formats'
require 'net/http'
require 'net/ftp'
require 'yaml'
require 'set'


class RunGpf < ProteomaticScript
	def run()
		@mk_Peptides = Set.new
		
		ls_GenomePath = ''
		if $gb_FixedGenomes
			ls_GenomePath = @param[:genome]
		else
			ls_GenomePath = @input[:genome].first
			unless fileMatchesFormat(ls_GenomePath, 'gpfindex')
				unless File::exists?(ls_GenomePath + '.monoisotopic-trypsin.gpfindex')
					puts 'The genome files you specified needs to be indexed now. This may take a while but only has to be done once.'
					ls_Command = "#{ExternalTools::binaryPath('gpf.gpfindex')} #{ls_GenomePath} \"#{File::basename(ls_GenomePath)}\""
					system(ls_Command)
				end
				ls_GenomePath = ls_GenomePath + '.monoisotopic-trypsin.gpfindex'
			end
		end
		
		def handlePeptide(as_Peptide, af_Mass = nil)
			ls_Id = "peptide=#{as_Peptide}"
			ls_Id += ";precursorMass=#{af_Mass}" if af_Mass
			@mk_Peptides.add(ls_Id)
		end
		
		@input[:predictions].each do |ls_Path|
			File.open(ls_Path, 'r') do |lk_File|
				lf_Mass = nil
				ls_Peptide = ''
				lk_File.each do |ls_Line|
					ls_Line.strip!
					if (ls_Line[0, 1] == '>')
						handlePeptide(ls_Peptide, lf_Mass) unless ls_Peptide.empty?
						lk_Line = ls_Line.split(' ')
						lf_Mass = nil
						if lk_Line.size > 3
							lf_Mass = Float(lk_Line[lk_Line.size - 3].strip)
							li_Charge = Integer(lk_Line[lk_Line.size - 2].strip)
							lf_Mass = lf_Mass * li_Charge - 1.007825 * (li_Charge - 1)
						end
						ls_Peptide = ''
					else
						ls_Peptide += ls_Line
					end
				end
				handlePeptide(ls_Peptide, lf_Mass) unless ls_Peptide.empty?
				lk_File.close()
			end
		end
		
		ls_Query = @mk_Peptides.to_a.sort.join("\n")
		
		ls_GpfOptions = "masses #{@param[:masses]} protease #{@param[:protease]} massError #{@param[:massError]} searchSimilar #{@param[:searchSimilar]} searchIntrons #{@param[:searchIntrons]} maxIntronLength #{@param[:maxIntronLength]} minChainLength #{@param[:minChainLength]} fullDetails yes"
		
		ls_QueryFile = tempFilename("gpf-queries-")
		File::open(ls_QueryFile, 'w') { |lk_File| lk_File.write(ls_Query) }
		
		ls_ResultFile = tempFilename("gpf-results-");
		
		ls_Command = "#{ExternalTools::binaryPath('gpf.gpfbatch')} #{ls_GpfOptions} #{ls_GenomePath} #{ls_QueryFile} #{ls_ResultFile}"
		unless system(ls_Command)
			puts 'There was an error while executing gpfbatch.'
			exit 1
		end
		
		FileUtils::cp(ls_ResultFile, @output[:gpfResults]) if @output[:gpfResults]
		
		lk_Hits = Set.new
		
		ls_LineBatch = ''
		File::open(ls_ResultFile, 'r').each_line do |ls_Line|
			if (ls_Line[0, 1] != ' ' && !ls_LineBatch.empty?)
				# handle line batch
				lk_ResultPart = YAML::load(ls_LineBatch)
				lk_ResultPart.each do |ls_Key, lk_Result|
					next unless lk_Result.class == Array
					next if lk_Result.empty?
					lk_Result.each { |lk_Hit| lk_Hits.add(lk_Hit['peptide']) }
				end
				ls_LineBatch = ''
			end
			ls_LineBatch += ls_Line
		end
		if (!ls_LineBatch.empty?)
			# handle line batch
			lk_ResultPart = YAML::load(ls_LineBatch)
			lk_ResultPart.each do |ls_Key, lk_Result|
				next unless lk_Result.class == Array
				next if lk_Result.empty?
				lk_Result.each { |lk_Hit| lk_Hits.add(lk_Hit['peptide']) }
			end
			ls_LineBatch = ''
		end
		
		puts "GPF found #{lk_Hits.size} hits."
		
		if @output[:gpfPeptides]
			File.open(@output[:gpfPeptides], 'w') do |lk_Out|
				lk_Hits.to_a.sort.each { |ls_Peptide| lk_Out.print ">gpf__#{ls_Peptide}\n#{ls_Peptide}\n" }
			end
		end
	end
end

lk_Object = RunGpf.new
