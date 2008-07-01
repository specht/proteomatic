require 'include/proteomatic'
require 'include/spectrum'
require 'yaml'
require 'fileutils'

class RunPeaks < ProteomaticScript
	def run()
		
		lk_SpectrumFiles = @input[:spectra]
		
		ls_PeaksConfig = ''
		ls_PeaksConfig += "<combine_result_files>1</combine_result_files>\n"
		ls_PeaksConfig += "<delete_temp>1</delete_temp>\n"
		ls_PeaksConfig += "<max_charge>2</max_charge>\n"
		ls_PeaksConfig += "<enzyme>Trypsin without PTMs</enzyme>\n"
		ls_PeaksConfig += "<frag_tol>1</frag_tol>\n"
		ls_PeaksConfig += "<instrument>-i</instrument>\n"
		ls_PeaksConfig += "<output_num>10</output_num>\n"
		ls_PeaksConfig += "<par_tol>1.5</par_tol>\n"
		
		ls_TempInDir = tempFilename('peaks-in')
		FileUtils::mkdir(ls_TempInDir)
		ls_MgfFile = File::join(ls_TempInDir, 'in.mgf')
		File.open(ls_MgfFile, 'w') do |lk_MgfFile|
			lk_SpectrumFiles.each do |ls_SpectrumFilename|
				lk_SpectrumProc = Proc.new { |ls_Contents| lk_MgfFile.write(ls_Contents) }
				MgfIterator.new(ls_SpectrumFilename, lk_SpectrumProc).run
			end
		end
		ls_ParamFile = tempFilename('peaks-config') + '.xml'
		File.open(ls_ParamFile, 'w') do |lk_File|
			lk_File.write(ls_PeaksConfig)
		end
		
		lf_PrecursorTolerance = @param[:precursorIonTolerance]
		lf_ProductTolerance = @param[:productIonTolerance]
		ls_Parameters = "-xf -i #{ls_TempInDir} . \"Trypsin without PTMs\" #{ls_ParamFile} #{lf_PrecursorTolerance} #{lf_ProductTolerance} 10 1"
		system("java -Xmx1024M -jar #{ExternalTools::getToolConfig('peaks.peaksbatch')['peaksBatchJar']} " + ls_Parameters)
		FileUtils::rm_rf(ls_TempInDir)
		
		#lk_Response = lk_PeaksDaemon.run({'peaks-params.xml' => ls_PeaksConfig, 'in\\peaks-remote.mgf' => ls_Spectrum}, ls_Parameters)
		
		
=begin
		ls_MgfFile = mergeSpectrumFilesToMgf(lk_SpectrumFiles, outputDirectory())
		
		lk_File = File.open(ls_MgfFile, 'r')
		ls_Spectrum = lk_File.read
		lk_File.close()
		
		DRb.start_service
		lk_PeaksDaemon = DRbObject.new_with_uri(param('daemonUrl'))
		puts 'Connection to PEAKS daemon established.'
		lf_PrecursorTolerance = param('precursorIonTolerance')
		lf_ProductTolerance = param('productIonTolerance')
		ls_Parameters = "-i in\\ . \"Trypsin without PTMs\" #{lf_PrecursorTolerance} #{lf_ProductTolerance} 10 1"
		lk_Response = lk_PeaksDaemon.run({'peaks-params.xml' => ls_PeaksConfig, 'in\\peaks-remote.mgf' => ls_Spectrum}, ls_Parameters)
		
		ls_Ticket = lk_Response['ticket']
		puts "Ticket: #{ls_Ticket}"
		
		li_Sleep = 1
		while true
			sleep li_Sleep
			li_Sleep += 1 if li_Sleep < 10
			break if lk_PeaksDaemon.finished?(ls_Ticket)
		end
		
		lk_Results = lk_PeaksDaemon.results(ls_Ticket)
		
		lk_Out = File.open(outFilename('resultFile'), 'w')
		lk_Out.write(lk_Results['files']['peaks-remote.fas'])
		lk_Out.close()
		
		FileUtils.rm([ls_MgfFile], :force => true)
		
		finishOutFiles()
=end
	end
end

lk_Object = RunPeaks.new


=begin
$gs_ScriptPath = 'd:/PeaksDaemon'
$gk_TempCounter = 101
$gk_Threads = Hash.new
$gk_Responses = Hash.new
$gk_ThreadsMutex = Mutex.new
$gk_ResponsesMutex = Mutex.new
$gk_JavaStartMutex = Mutex.new

class PeaksDaemon

	def initialize()
	end
	
	# start a PEAKS process, return a ticket
	def run(ak_Files, as_Arguments)
		puts "Incoming PEAKS request:"
		lk_FileLabels = Array.new
		ak_Files.each { |ls_Filename, ls_Contents| lk_FileLabels.push("#{ls_Filename} (#{ls_Contents.size} bytes)") }
		puts "files: #{lk_FileLabels.join(', ')}"
		puts "arguments: #{as_Arguments}"
		
		$gk_TempCounter += 1
		ls_Ticket = "pd#{$gk_TempCounter.to_s(36)}"
		puts "ticket: #{ls_Ticket}"
		
		FileUtils.rmtree("pdtemp/#{ls_Ticket}")
		FileUtils.mkpath("pdtemp/#{ls_Ticket}")
		ak_Files.each do |ls_Filename, ls_Contents|
			FileUtils.mkpath("pdtemp/#{ls_Ticket}/#{File.dirname(ls_Filename)}")
			lk_File = File.open("pdtemp/#{ls_Ticket}/#{ls_Filename}", 'w')
			lk_File.write(ls_Contents)
			lk_File.close()
		end
		$gk_ThreadsMutex.synchronize do
			$gk_Threads[ls_Ticket] = Thread.new(ls_Ticket, ak_Files.keys, as_Arguments) { |tls_Ticket, tls_FileKeys, tls_Arguments|
				lk_Stream = nil
				$gk_JavaStartMutex.synchronize do
					Dir.chdir("pdtemp/#{tls_Ticket}")
					lk_Stream = IO.popen("javaw -Xmx512M -jar d:\\tools\\PEAKS\\peaksbatch_V24_040818.jar " + tls_Arguments, "r")
					Dir.chdir($gs_ScriptPath)
				end
				ls_Response = lk_Stream.read if (!lk_Stream.eof?)
				lk_FilePaths = Dir.glob("pdtemp/#{tls_Ticket}/*")
				lk_Files = Hash.new
				lk_FilePaths.each do |ls_Path| 
					ls_Filename = ls_Path.sub("pdtemp/#{tls_Ticket}/", '')
					if (!tls_FileKeys.include?(ls_Filename) && File.file?(ls_Path))
						lk_File = File.open(ls_Path, 'r')
						ls_Contents = lk_File.read
						lk_File.close()
						lk_Files[ls_Filename] = ls_Contents
					end
				end
				$gk_ResponsesMutex.synchronize do
					$gk_Responses[tls_Ticket] = {'response' => ls_Response, 'files' => lk_Files}
				end
			}
		end
		return { 'ticket' => ls_Ticket }
	end
	
	# check whether a PEAKS process with the specified ticket has already finished
	def finished?(as_Ticket)
		$gk_ThreadsMutex.synchronize do
			return $gk_Threads[as_Ticket].stop?
		end
	end
	
	def results(as_Ticket)
		return if !finished?(as_Ticket)
		$gk_ThreadsMutex.synchronize do
			if $gk_Threads.has_key?(as_Ticket)
				$gk_Threads.delete(as_Ticket)
				FileUtils.rmtree("pdtemp/#{as_Ticket}")
			end
		end
		$gk_ResponsesMutex.synchronize do
			lk_Response = $gk_Responses[as_Ticket]
			$gk_Responses.delete(as_Ticket)
			return lk_Response
		end
	end
end

Dir.chdir($gs_ScriptPath)
FileUtils.mkpath("pdtemp/")
DRb.start_service("druby://128.176.144.92:19811", PeaksDaemon.new)
puts DRb.uri
DRb.thread.join
=end