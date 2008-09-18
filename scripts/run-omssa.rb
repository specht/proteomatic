require 'include/proteomatic'
require 'include/externaltools'
require 'include/fasta'
require 'yaml'
require 'fileutils'

class RunOmssa < ProteomaticScript

	def runOmssa(as_SpectrumFilename, ak_Databases, as_OutputDirectory, as_Format = 'csv')
		print "#{File::basename(as_SpectrumFilename)}:"
		
		@lk_TargetDecoyFilenames = Array.new()
		ak_Databases.each { |ls_Path| @lk_TargetDecoyFilenames.push(getTargetDecoyFilename(ls_Path)) }
		@lk_OutFiles = Array.new()
		
		@ls_OutputDirectory = as_OutputDirectory
		@ls_SpectrumFilename = as_SpectrumFilename
		@ls_MgfFilename = tempFilename("mgf-#{File.basename(@ls_SpectrumFilename)}")
		
		@lk_MgfFile = File.open(@ls_MgfFilename, 'w')
		@li_SpectrumBatchSize = 0
		@lb_FirstBatch = true
		
		@li_TotalSpectraCount = 0
		
		@ls_SpectrumFilename = as_SpectrumFilename
		
		def handleSpectrumBatch()
			@li_TotalSpectraCount += @li_SpectrumBatchSize
			print "\r#{File::basename(@ls_SpectrumFilename)}: #{@li_TotalSpectraCount} spectr#{@li_TotalSpectraCount == 1 ? 'um' : 'a'} "
		
			@lk_MgfFile.close()
			
			lb_TestDtaExisted = File.exists?('test.dta')
			
			@lk_TargetDecoyFilenames.each do |ls_TargetDecoyPath|
				ls_OutFilename = tempFilename("omssa-out-#{File.basename(@ls_SpectrumFilename)}")
				@lk_OutFiles.push(ls_OutFilename)
				ls_Command = "\"#{ExternalTools::binaryPath('omssa.omssacl')}\" -d \"#{ls_TargetDecoyPath}\" -fm \"#{@ls_MgfFilename}\" -oc \"#{ls_OutFilename}\" -ni "
				ls_Command += @mk_Parameters.commandLineFor('omssa.omssacl')
				
				system(ls_Command);
			end
	
			File::delete('test.dta') if File.exists?('test.dta') && !lb_TestDtaExisted
			
			@li_SpectrumBatchSize = 0
			begin
				File::delete(@lk_MgfFile.path)
			rescue StandardError => e
				puts e
			end
			@lk_MgfFile = File.open(@ls_MgfFilename, 'w')
			@lb_FirstBatch = false
		end
		
		lk_SpectrumProc = Proc.new do |ls_Contents|
			@lk_MgfFile.write(ls_Contents)
			@li_SpectrumBatchSize += 1
			handleSpectrumBatch() if (@li_SpectrumBatchSize >= @param[:batchSize])
		end
		
		MgfIterator.new(as_SpectrumFilename, lk_SpectrumProc, :levels => 2, :iterateAllCharges => @param['omssa.omssacl.precursorChargeDetermination'.intern] == 1).run
		handleSpectrumBatch()
		
		puts
		
		@lk_MgfFile.close()
		begin
			File::delete(@lk_MgfFile.path)
		rescue StandardError => e
			puts e
		end
		
		return @lk_OutFiles
	end


	def run()
		# merge all input databases into one database
		lb_MergedDatabase = false
		lk_Databases = Array.new
		if @input[:databases].size == 1
			lk_Databases = @input[:databases]
		else
			puts 'Merging databases...'
			lb_MergedDatabase = true
			ls_MergedDatabase = tempFilename('merged-database');
			File::open(ls_MergedDatabase, 'w') do |lk_OutFile|
				@input[:databases].each do |ls_Path|
					File::open(ls_Path, 'r') do |lk_InFile|
						ls_Contents = lk_InFile.read
						ls_Contents += "\n" unless ls_Contents[-1] == "\n"
						lk_OutFile.write(ls_Contents)
					end
				end
			end
			lk_Databases = [ls_MergedDatabase]
		end
		
		puts 'Creating target-decoy database and converting to BLAST format...'
		# ensure that all databases are target-decoy and in BLAST format
		lk_ToBeDeleted = Array.new
		lk_Databases.each do |ls_Path|
			createTargetDecoyDatabase(ls_Path)
			createBlastDatabase(getTargetDecoyFilename(ls_Path))
			if (lb_MergedDatabase)
				lk_ToBeDeleted.push(getTargetDecoyFilename(ls_Path))
				lk_ToBeDeleted.push(getTargetDecoyFilename(ls_Path) + '.phr')
				lk_ToBeDeleted.push(getTargetDecoyFilename(ls_Path) + '.psq')
				lk_ToBeDeleted.push(getTargetDecoyFilename(ls_Path) + '.pin')
			end
		end
		
		# convert spectra to MGF
		ls_MgfTempPath = tempFilename('mgf-temp')
		FileUtils::mkpath(ls_MgfTempPath)
		system("\"#{ExternalTools::binaryPath('xml2mgf.xml2mgf')}\" -b 2000 -o \"#{ls_MgfTempPath}/out\" #{@input[:spectra].join(' ')}");
		
		# run OMSSA on each spectrum
=begin		
		lk_OutFiles = Array.new
		@input[:spectra].each do |ls_Path|
			lk_OutFiles += runOmssa(ls_Path, lk_Databases, File::dirname(@output[:resultFile]), 'csv')
		end
		
		
		# merge results
		print "Merging OMSSA results..."
		mergeCsvFiles(lk_OutFiles, @output[:resultFile])
=end		
		FileUtils.rm_f(lk_ToBeDeleted);
		puts "done."
	end
end

lk_Object = RunOmssa.new
=begin
def mergeSpectrumFilesToMgf(ak_SpectrumFilenames, as_OutputDirectory)
	@ls_OutputDirectory = as_OutputDirectory
	@ls_MgfFilename = tempFilename()
	
	@lk_MgfFile = File.open(@ls_MgfFilename, 'w')
	lk_SpectrumProc = Proc.new { |ls_Contents| @lk_MgfFile.write(ls_Contents) }
	ak_SpectrumFilenames.each { |ls_Filename| MgfIterator.new(ls_Filename, lk_SpectrumProc).run }
	
	@lk_MgfFile.close()
	return  @ls_MgfFilename
end
=end
