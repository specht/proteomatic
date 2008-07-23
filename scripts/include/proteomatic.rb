require 'include/externaltools'
require 'include/formats'
require 'include/misc'
require 'include/parameters'
require 'drb'
require 'fileutils'
require 'set'
require 'tempfile'
require 'thread'
require 'yaml'
require 'zlib'


class ProteomaticScriptDaemon
	def initialize(ak_Script)
		@mk_Script = ak_Script
		@ms_ScriptHelp = @mk_Script.help()
		@ms_ScriptInfo = @mk_Script.info()
		@ms_ScriptGetParameters = @mk_Script.getParameters()
		
		@mk_Tickets = Hash.new
		@mk_TicketOrder = Array.new
		@mk_TicketsMutex = Mutex.new

		# set up paths
		@ms_ScriptPath = File.expand_path(File.join(File.dirname(__FILE__), '..'))
		@ms_TempPath = File.join(@ms_ScriptPath, 'jobs', File.basename($0).sub!('.rb', ''))
		FileUtils.mkpath(@ms_TempPath)
		
		# find pending jobs
		lk_Pending = Dir::glob(@ms_TempPath + '/*').collect { |x| File.basename(x) }
		lk_Pending.each do |ls_Ticket|
			ls_State = nil
			ls_State = :waiting if File::exists?(File.join(@ms_TempPath, ls_Ticket, 'waiting'))
			ls_State = :finished if File::exists?(File.join(@ms_TempPath, ls_Ticket, 'finished'))
			if File::exists?(File.join(@ms_TempPath, ls_Ticket, 'running'))
				# revert job if it was running while the daemon was shut down
				FileUtils::mv(File.join(@ms_TempPath, ls_Ticket, 'running'), File.join(@ms_TempPath, ls_Ticket, 'waiting'))
				ls_State = :waiting
			end
			@mk_Tickets[ls_Ticket] = ls_State if ls_State
			@mk_TicketOrder.push(ls_Ticket) if ls_State == :waiting
		end
		@mk_TicketOrder.sort! do |x, y|
			File::ctime(File::join(@ms_TempPath, x, 'arguments.yaml')) <=> File::ctime(File::join(@ms_TempPath, y, 'arguments.yaml'))
		end
		
		# start worker thread
		@mk_WorkerThread = Thread.new do
			while true
				ls_NextTicket = nil
				@mk_TicketsMutex.synchronize { ls_NextTicket = @mk_TicketOrder.first }
				
				# go to sleep if no more tickets
				Thread.stop unless ls_NextTicket

				if ls_NextTicket
					# fetch and handle next job
					
					# switch to running state
					@mk_TicketsMutex.synchronize { @mk_Tickets[ls_NextTicket] = :running }
					FileUtils::mv(File.join(@ms_TempPath, ls_NextTicket, 'waiting'), File.join(@ms_TempPath, ls_NextTicket, 'running'))
					
					# remove out files, if any (in case the job was once running and then aborted)
					lk_OutFiles = Dir::glob(File::join(@ms_TempPath, ls_NextTicket, 'out', '*'))
					FileUtils::rm_f(lk_OutFiles)
					
					# load and apply arguments
					lk_Arguments = YAML::load_file(File::join(@ms_TempPath, ls_NextTicket, 'arguments.yaml'))
					lk_Arguments = lk_Arguments.select { |x| x[0, 14] != '----ignore----' }
					lk_Arguments.push(File::join(@ms_TempPath, ls_NextTicket, 'out'))
					lb_Exception = false
					$stdout = File.new(File::join(@ms_TempPath, ls_NextTicket, 'stdout.txt'), 'w')
					$stderr = File.new(File::join(@ms_TempPath, ls_NextTicket, 'stderr.txt'), 'w')
					begin
						@mk_Script.applyArguments(lk_Arguments)
					rescue ProteomaticArgumentException => e
						puts e
						lb_Exception = true
					end
					$stdout.close()
					$stderr.close()
					$stdout = STDOUT
					$stderr = STDERR
					
					# execute job if arguments were good
					unless lb_Exception
						$stdout = File.new(File::join(@ms_TempPath, ls_NextTicket, 'stdout.txt'), 'a')
						$stderr = File.new(File::join(@ms_TempPath, ls_NextTicket, 'stderr.txt'), 'a')
						@mk_Script.run()
						lk_Files = YAML::load_file(File.join(@ms_TempPath, ls_NextTicket, 'output-files.yaml'))
						lk_Files['files'] = @mk_Script.finishOutputFiles()
						lk_Mapping = YAML::load_file(File::join(@ms_TempPath, ls_NextTicket, 'file-mapping.yaml'))
						ls_Path = lk_Mapping[@mk_Script.fileDefiningOutputDirectory()]
						lk_Files['directory'] = File::dirname(ls_Path) if (!lk_Files.has_key?('directory')) && ls_Path
						lk_Files['prefix'] = @mk_Script.param('[output]prefix')
						File.open(File.join(@ms_TempPath, ls_NextTicket, 'output-files.yaml'), 'w') do |lk_File| 
							lk_File.puts lk_Files.to_yaml
						end
						$stdout.close()
						$stderr.close()
						$stdout = STDOUT
						$stderr = STDERR
					end
					
					# switch to finished state
					@mk_TicketsMutex.synchronize do
						@mk_Tickets[ls_NextTicket] = :finished
						@mk_TicketOrder.slice!(0)
					end
					FileUtils::mv(File.join(@ms_TempPath, ls_NextTicket, 'running'), File.join(@ms_TempPath, ls_NextTicket, 'finished'))
				end
			end
		end
		@mk_WorkerThread.priority = -1
	end
	
	def proteomatic?()
		return 'proteomatic!'
	end

	def help()
		return @ms_ScriptHelp
	end

	def getParameters()
		return @ms_ScriptGetParameters
	end
	
	def info()
		return @ms_ScriptInfo
	end

	def infoAndParameters()
		return @ms_ScriptInfo + @ms_ScriptGetParameters
	end

	def submit(ak_Arguments, ak_Files, as_OutputDirectory)
		ls_Ticket = nil
		ls_Letters = 'bcdfghjkmpqrstvwxyz'
		ls_Digits = '0123456789'
		@mk_TicketsMutex.synchronize do
			while ls_Ticket == nil || @mk_Tickets.has_key?(ls_Ticket)
				ls_Ticket = ''
				ls_Ticket += ls_Letters[rand(ls_Letters.size), 1]
				ls_Ticket += ls_Digits[rand(ls_Digits.size), 1]
				ls_Ticket += ls_Digits[rand(ls_Digits.size), 1]
				ls_Ticket += ls_Letters[rand(ls_Letters.size), 1]
			end
			@mk_Tickets[ls_Ticket] = :waiting
		end
		
		FileUtils.mkpath(File.join(@ms_TempPath, ls_Ticket, 'in'))
		FileUtils.mkpath(File.join(@ms_TempPath, ls_Ticket, 'out'))
		lk_FileCount = Hash.new
		lk_FileMapping = Hash.new
		lb_FilesMissing = false
		ak_Files.each do |ls_Path, ls_Contents|
			ls_BaseName = File::basename(ls_Path)
			lk_FileCount[ls_BaseName] ||= -1
			lk_FileCount[ls_BaseName] += 1
			ls_SandboxPath = File.join(@ms_TempPath, ls_Ticket, 'in', lk_FileCount[ls_BaseName].to_s + '-' + ls_BaseName)
			ls_SandboxPath = File.join(@ms_TempPath, ls_Ticket, 'in', ls_BaseName) if (lk_FileCount[ls_BaseName] == 0)
			if ls_Contents == nil
				lb_FilesMissing = true
				File.open(ls_SandboxPath, 'wb') { |lk_File| }
			else
				File.open(ls_SandboxPath, 'wb') { |lk_File| lk_File.write(ls_Contents) }
			end
			ak_Arguments.push(ls_SandboxPath)
			lk_FileMapping[ls_SandboxPath] = ls_Path
		end
		File.open(File.join(@ms_TempPath, ls_Ticket, 'arguments.yaml'), 'w') { |lk_File| lk_File.puts ak_Arguments.to_yaml }
		File.open(File.join(@ms_TempPath, ls_Ticket, 'file-mapping.yaml'), 'w') { |lk_File| lk_File.puts lk_FileMapping.to_yaml }
		lk_OutputFileInfo = Hash.new
		lk_OutputFileInfo['directory'] = as_OutputDirectory if as_OutputDirectory
		File.open(File.join(@ms_TempPath, ls_Ticket, 'output-files.yaml'), 'w') { |lk_File| lk_File.puts lk_OutputFileInfo.to_yaml }
		unless lb_FilesMissing
			File.open(File.join(@ms_TempPath, ls_Ticket, 'waiting'), 'w') 
			@mk_TicketsMutex.synchronize { @mk_TicketOrder.push(ls_Ticket) }
			@mk_WorkerThread.wakeup
		end
		return ls_Ticket
	end
	
	def submitInputFileChunk(as_Ticket, as_Path, as_Chunk)
		lk_ReverseFileMapping = YAML::load_file(File.join(@ms_TempPath, as_Ticket, 'file-mapping.yaml')).invert
		ls_SandboxPath = lk_ReverseFileMapping[as_Path]
		File::open(ls_SandboxPath, 'ab') { |lk_File| lk_File.write(Zlib::Inflate.inflate(as_Chunk)) }
		#sleep 0.1
	end
	
	def submitInputFilesFinished(as_Ticket)
		File.open(File.join(@ms_TempPath, as_Ticket, 'waiting'), 'w') 
		@mk_TicketsMutex.synchronize { @mk_TicketOrder.push(as_Ticket) }
		@mk_WorkerThread.wakeup
	end
	
	def queryTicket(as_Ticket)
		lk_Info = Hash.new
		@mk_TicketsMutex.synchronize do
			lk_Info['state'] = @mk_Tickets[as_Ticket].to_s
			lk_Info['state'] ||= 'unknown'
			if @mk_Tickets[as_Ticket] == :waiting
				lk_Info['infront'] = @mk_TicketOrder.index(as_Ticket)
			end
			if @mk_Tickets[as_Ticket] == :finished
				lk_OutputFileInfo = YAML::load(File.read(File.join(@ms_TempPath, as_Ticket, 'output-files.yaml')))
				lk_Info['output'] = lk_OutputFileInfo
				
				# ensure default values
				lk_Info['output']['directory'] ||= ''
				lk_Info['output']['prefix'] ||= ''
				lk_Info['output']['files'] ||= []
			end
		end
		return lk_Info
	end
	
	def finished?(as_Ticket)
		lb_Finished = false
		@mk_TicketsMutex.synchronize { lb_Finished = (@mk_Tickets[as_Ticket] == :finished) }
		return lb_Finished
	end
	
	def outputFileInfo(as_Ticket)
		@mk_TicketsMutex.synchronize { return nil unless @mk_Tickets[as_Ticket] == :finished }
		ls_Path = File::join(@ms_TempPath, as_Ticket, 'output-files.yaml')
		return [] unless File::exists?(ls_Path)
		return YAML::load_file(ls_Path)
	end
	
	def getOutputFile(as_Ticket, as_Filename)
		@mk_TicketsMutex.synchronize { return nil unless @mk_Tickets[as_Ticket] == :finished }
		return File::read(File::join(@ms_TempPath, as_Ticket, 'out', as_Filename))
	end
	
	def getStandardOutput(as_Ticket)
		@mk_TicketsMutex.synchronize { return nil unless @mk_Tickets[as_Ticket] == :finished }
		return File::read(File::join(@ms_TempPath, as_Ticket, 'stdout.txt'))
	end

	def getStandardError(as_Ticket)
		@mk_TicketsMutex.synchronize { return nil unless @mk_Tickets[as_Ticket] == :finished }
		return File::read(File::join(@ms_TempPath, as_Ticket, 'stderr.txt'))
	end
end


class ProteomaticArgumentException < StandardError
end


class ProteomaticScript
	def initialize()
	
		@mk_TempFiles = Array.new
	
		# flush stdout and stderr every second... TODO: find a better way
		@mk_FlushThread = Thread.new do
			while true
				$stdout.flush
				$stderr.flush
				sleep 1.0
			end
		end
		
		@ms_Platform = determinePlatform()
		@ms_Platform.freeze
		
		loadDescription()
		
		if ARGV == ['--resolveDependencies']
			resolveDependencies()
			exit 0
		end
		
		if (@mb_NeedsConfig && !@mb_HasConfig)
			puts "Error: This script needs a configuration file. Please check the config directory for a template and instructions."
			exit 1
		end
		
		unless ARGV == ['---info']
			begin
				setupParameters()
			rescue ProteomaticArgumentException => e
				puts e
				exit 1
			end
		end
		handleArguments()
		
		if @mb_Daemon
			resolveDependencies()
			DRb.start_service(@ms_DaemonUri, ProteomaticScriptDaemon.new(self))
			puts "#{@ms_Title} daemon listening at #{DRb.uri}"
			DRb.thread.join
		else
			begin
				applyArguments(ARGV)
			rescue ProteomaticArgumentException => e
				puts e
				exit 1
			end
			run()
			finishOutputFiles()
		end
	end
	
	def help()
		ls_Result = ''
		ls_Result += "#{underline(@ms_Title + ' (a Proteomatic script)', '=')}\n"
		ls_Result += indent(wordwrap("Description: #{@ms_Description}\n"), 4, false) + "\n" unless @ms_Description.empty?
		if @mk_Input
			ls_Result += "#{underline('Input files', '-')}\n"
			@mk_Input['groupOrder'].each do |ls_Group|
				ls_Range = ''
				ls_Range += 'min' if @mk_Input['groups'][ls_Group]['min']
				ls_Range += 'max' if @mk_Input['groups'][ls_Group]['max']
				ls_Result += '- '
				ls_FileLabel = ''
				if (ls_Range == 'min')
					ls_Result += "at least #{@mk_Input['groups'][ls_Group]['min']} "
					ls_FileLabel = "file#{@mk_Input['groups'][ls_Group]['min'] != 1 ? 's' : ''} "
				elsif (ls_Range == 'max')
					ls_Result += "at most #{@mk_Input['groups'][ls_Group]['max']} "
					ls_FileLabel = "file#{@mk_Input['groups'][ls_Group]['max'] != 1 ? 's' : ''} "
				elsif (ls_Range == 'minmax')
					if (@mk_Input['groups'][ls_Group]['min'] == @mk_Input['groups'][ls_Group]['max'])
						ls_Result += "exactly #{@mk_Input['groups'][ls_Group]['min']} "
						ls_FileLabel = "file#{@mk_Input['groups'][ls_Group]['min'] != 1 ? 's' : ''} "
					else
						ls_Result += "at least #{@mk_Input['groups'][ls_Group]['min']}, but no more than #{@mk_Input['groups'][ls_Group]['max']} "
						ls_FileLabel = 'files '
					end
				end
				ls_Result += 'optional: ' if (ls_Range == '')
				ls_Result += "#{@mk_Input['groups'][ls_Group]['label']} #{ls_FileLabel}"
				ls_Result += "\n"
				ls_Result += "  format: #{@mk_Input['groups'][ls_Group]['formats'].collect { |x| info = formatInfo(x); "#{info['description']} (#{info['extensions'].join('|')})" }.join(', ')}\n"
				ls_Result += "\n"
			end
		end
		ls_Result += @mk_Parameters.helpString()
		return ls_Result
	end
	
	def getParameters()
		ls_Result = ''
		ls_Result << "---getParameters\n"
		ls_Result << @mk_Parameters.parametersString()
		if @mk_Input
			ls_Result << "!!!begin input\n"
			@mk_Input['groupOrder'].each do |ls_Group|
				ls_Format = "#{@mk_Input['groups'][ls_Group]['formats'].collect { |x| formatInfo(x)['extensions'] }.flatten.uniq.sort.join('|')}"
				ls_Range = ''
				ls_Range += 'min' if @mk_Input['groups'][ls_Group]['min']
				ls_Range += 'max' if @mk_Input['groups'][ls_Group]['max']
				ls_FileLabel = ''
				if (ls_Range == 'min')
					ls_Result += "at least #{@mk_Input['groups'][ls_Group]['min']} "
					ls_FileLabel = "file#{@mk_Input['groups'][ls_Group]['min'] != 1 ? 's' : ''} "
				elsif (ls_Range == 'max')
					ls_Result += "at most #{@mk_Input['groups'][ls_Group]['max']} "
					ls_FileLabel = "file#{@mk_Input['groups'][ls_Group]['max'] != 1 ? 's' : ''} "
				elsif (ls_Range == 'minmax')
					if (@mk_Input['groups'][ls_Group]['min'] == @mk_Input['groups'][ls_Group]['max'])
						ls_Result += "exactly #{@mk_Input['groups'][ls_Group]['min']} "
						ls_FileLabel = "file#{@mk_Input['groups'][ls_Group]['min'] != 1 ? 's' : ''} "
					else
						ls_Result += "at least #{@mk_Input['groups'][ls_Group]['min']}, but no more than #{@mk_Input['groups'][ls_Group]['max']} "
						ls_FileLabel = 'files '
					end
				end
				ls_Result += 'optional: ' if (ls_Range == '')
				ls_Result += "#{@mk_Input['groups'][ls_Group]['label']} #{ls_FileLabel}"
				ls_Result += "(#{ls_Format})"
				ls_Result += "\n"
			end
			ls_Result << "!!!end input\n"
		end
		return ls_Result
	end
	
	def info()
		ls_Result = ''
		ls_Result << "---info\n"
		ls_Result << "#{@ms_Title}\n"
		ls_Result << "#{@ms_Group}\n"
		ls_Result << "#{@ms_Description}\n"
		return ls_Result
	end
	
	def handleArguments()
		if ARGV.first == '--daemon'
			@mb_Daemon = true
			if ARGV.size < 2
				puts 'Error: No daemon URI specified!'
				exit 1
			end
			@ms_DaemonUri = ARGV[1]
		end
		if ARGV == ['---getParameters']
			puts getParameters()
			exit 0
		end
		if ARGV == ['--help']
			puts help()
			exit 0
		end		
		if ARGV == ['---info']
			puts info()
			exit 0
		end
	end
	private :handleArguments

	def resolveDependencies()
		@mk_ScriptProperties['needs'].each do |ls_ExtTool|
			# skip if 'config' and not a proper 'package.program' tool
			next unless ls_ExtTool[0, 4] == 'ext.'
			ExternalTools::install(ls_ExtTool) unless ExternalTools::installed?(ls_ExtTool)
		end
	end
	private :resolveDependencies
	
	def loadDescription()
		# load script parameters and external tools
		@ms_ScriptName = File.basename($0).sub('.defunct.', '.').sub('.rb', '')
		@mk_ScriptProperties = YAML::load_file("include/properties/#{@ms_ScriptName}.yaml")
		unless @mk_ScriptProperties.has_key?('title')
			puts 'Internal error: Script has no title.'
			exit 1
		end
		unless @mk_ScriptProperties.has_key?('group')
			puts 'Internal error: Script has no group.'
			exit 1
		end
		@ms_ScriptType = nil
		@ms_ScriptType = @mk_ScriptProperties['type']
		unless (@ms_ScriptType == 'processor' || @ms_ScriptType == 'converter')
			puts 'Internal error: No type or invalid type defined for this script.'
			exit 1
		end
		@mk_ScriptProperties.default = ''
		@ms_Description = @mk_ScriptProperties['description']
		@ms_Description.freeze
		@ms_Title = @mk_ScriptProperties['title']
		@ms_Title.freeze
		@ms_Group = @mk_ScriptProperties['group']
		@ms_Group.freeze
		if @mk_ScriptProperties['needs'] && @mk_ScriptProperties['needs'].include?('config')
			@mb_NeedsConfig = true
		else
			@mb_NeedsConfig = false
		end
		@mb_HasConfig = File::exists?(File::join('config', "#{@ms_ScriptName}.config.yaml"))
	end
	private :loadDescription
	
	def setupParameters()
		lk_Errors = Array.new
		
		# check dependencies if called from GUI
		if @mk_ScriptProperties.has_key?('needs')
			if ARGV == ['---getParameters'] 
				ls_Response = ''
				@mk_ScriptProperties['needs'].each do |ls_ExtTool|
					next unless ls_ExtTool[0, 4] == 'ext.'
					ls_Response << "#{ExternalTools::packageTitle(ls_ExtTool)}\n" unless ExternalTools::installed?(ls_ExtTool)
				end
				unless ls_Response.empty?
					puts '---getParametersUnresolvedDependencies'
					puts ls_Response
					exit 1
				end
			end
		end
		
		raise ProteomaticArgumentException, "Error#{lk_Errors.size > 1 ? "s:\n": ": "}" + lk_Errors.join("\n") unless lk_Errors.empty?
		lk_Errors = Array.new
		
		@mk_Parameters = Parameters.new

		# add script parameters
		@mk_ScriptProperties['parameters'].each { |lk_Parameter| @mk_Parameters.addParameter(lk_Parameter) }
		
		# add external tool parameters if desired
		if (@mk_ScriptProperties.has_key?('externalParameters'))
			@mk_ScriptProperties['externalParameters'].each do |ls_ExtTool|
				lk_Properties = YAML::load_file("include/properties/ext.#{ls_ExtTool}.yaml")
				lk_Properties['parameters'].each do |lk_Parameter| 
					lk_Parameter['key'] = ls_ExtTool + '.' + lk_Parameter['key']
					@mk_Parameters.addParameter(lk_Parameter, ls_ExtTool)
				end
			end
		end
		
		# handle input files
		lk_InputFormats = Hash.new
		lk_InputGroups = Hash.new
		lk_InputGroupOrder = Array.new
		@mk_ScriptProperties['input'].each do |lk_InputGroup|
			unless lk_InputGroup.has_key?('key')
				puts "Internal error: Input group has no key."
				exit 1
			end
			unless lk_InputGroup['formats'].class == Array
				puts "Internal error: 'formats' must be an array."
				exit 1
			end
			lk_InputGroups[lk_InputGroup['key']] = lk_InputGroup
			lk_InputGroupOrder.push(lk_InputGroup['key'])
			lk_InputGroup['formats'].each do |ls_Format|
				assertFormat(ls_Format)
				if lk_InputFormats.has_key?(ls_Format)
					puts "Internal error: #{ls_Format} appears in more than one input file group."
					exit 1
				end
				lk_InputFormats[ls_Format] = lk_InputGroup['key']
			end
		end
		@mk_Input = Hash.new
		@mk_Input['groups'] = lk_InputGroups
		@mk_Input['groupOrder'] = lk_InputGroupOrder
		@mk_Input.freeze

		# handle output files
		if @mk_ScriptProperties.has_key?('output')
			lk_Directory = {'group' => 'Output files', 'key' => '[output]directory', 
				'label' => 'Output directory', 'type' => 'string', 'default' => '', 'colspan' => 2}
			@mk_Parameters.addParameter(lk_Directory)
			lk_Prefix = {'group' => 'Output files', 'key' => '[output]prefix', 
				'label' => 'Output file prefix', 'type' => 'string', 'default' => '', 'colspan' => 2}
			@mk_Parameters.addParameter(lk_Prefix)
		end

		lk_OutputFiles = Hash.new
		@mk_ScriptProperties['output'].each do |lk_OutputFile|
			unless lk_OutputFile.has_key?('key')
				puts "Internal error: Output file has no key."
				exit 1
			end
			ls_Key = lk_OutputFile['key']
			ls_Label = lk_OutputFile['label']
			lk_OutputFiles[ls_Key] = lk_OutputFile
			assertFormat(lk_OutputFile['format'])
			if (@ms_ScriptType == 'processor')
				ls_Key = lk_OutputFile['key']
				ls_Key[0, 1] = ls_Key[0, 1].upcase
				lk_WriteFlag = {'group' => 'Output files', 'key' => "[output]write#{ls_Key}",
					'label' => "Write #{ls_Label}", 'type' => 'flag',
					'filename' => lk_OutputFile['filename']}
				if (lk_OutputFile.has_key?('force'))
					lk_WriteFlag['force'] = lk_OutputFile['force'] == true ? 'yes' : 'no'
					lk_WriteFlag['default'] = lk_OutputFile['force'] == true ? 'yes' : 'no'
				else
					lk_WriteFlag['default'] = lk_OutputFile['default'] == true ? 'yes' : 'no'
				end
				lk_WriteFlag['description'] = "Write #{ls_Label} (#{lk_WriteFlag['filename']})"
				@mk_Parameters.addParameter(lk_WriteFlag)
			end
		end
		@mk_Output = lk_OutputFiles
		@mk_Output.freeze

		# check if there's a default output directory if output files are to be written
		if @mk_ScriptProperties.has_key?('output')
			if !@mk_ScriptProperties.has_key?('defaultOutputDirectory')
				puts "Internal error: No default output directory specified for this script."
				exit 1
			end
			@ms_DefaultOutputDirectoryGroup = @mk_ScriptProperties['defaultOutputDirectory']
			if !@mk_Input['groups'].has_key?(@ms_DefaultOutputDirectoryGroup)
				puts "Internal error: Invalid default output directory specified for this script."
				exit 1
			end
		end
		raise ProteomaticArgumentException, "Error#{lk_Errors.size > 1 ? "s:\n": ": "}" + lk_Errors.join("\n") unless lk_Errors.empty?
	end
	private :loadDescription

	def applyArguments(ak_Arguments)
		lk_Arguments = ak_Arguments.dup

		# digest parameters
		@mk_Parameters.keys().each { |ls_Key| @mk_Parameters.reset(ls_Key) }
		@mk_Parameters.applyParameters(lk_Arguments)
		@param = Hash.new
		@mk_Parameters.keys().each { |ls_Key| @param[ls_Key.intern] = @mk_Parameters.value(ls_Key) }

		lk_Files = lk_Arguments.select { |ls_Path| File::file?(ls_Path) }
		lk_Arguments -= lk_Files
		lk_Directories = [@param['[output]directory'.intern]]
		lk_Directories = Array.new if @param['[output]directory'.intern].empty?

		lk_Errors = Array.new
		
		# determine input files
		@input = Hash.new
		lk_Files.each do |ls_Path|
			ls_Group = findGroupForFile(ls_Path)
			if (ls_Group)
				@input[ls_Group.intern] ||= Array.new
				@input[ls_Group.intern].push(ls_Path)
			end
		end

		# check input files min/max conditions
		@mk_Input['groups'].each do |ls_Group, lk_Group|
			ls_Format = "#{@mk_Input['groups'][ls_Group]['formats'].collect { |x| formatInfo(x)['extensions'] }.flatten.uniq.sort.join('|')}"
			li_Min = lk_Group['min']
			if li_Min && (!@input.has_key?(ls_Group.intern) || @input[ls_Group.intern].size < li_Min)
				lk_Errors.push("At least #{li_Min} #{lk_Group['label']} file#{li_Min == 1 ? " (#{ls_Format}) is" : "s #{ls_Format} are"} required.")
			end
			li_Max = lk_Group['max']
			if li_Max && @input.has_key?(ls_Group.intern) && @input[ls_Group.intern].size > li_Max
				lk_Errors.push("At most #{li_Min} #{lk_Group['label']} file#{li_Min == 1 ? " (#{ls_Format}) is" : "s (#{ls_Format}) are"} allowed.")
			end
		end

		# determine output files
		@output = Hash.new
		ls_OutputDirectory = nil
		@ms_FileDefiningOutputDirectory = nil
		unless @ms_DefaultOutputDirectoryGroup == nil || @input.empty?
			if @input[@ms_DefaultOutputDirectoryGroup.intern].class == Array
				@ms_FileDefiningOutputDirectory = @input[@ms_DefaultOutputDirectoryGroup.intern].first 
			end
		end
		
		unless lk_Directories.empty?
			ls_OutputDirectory = lk_Directories.first
		else
			ls_OutputDirectory = File::dirname(@ms_FileDefiningOutputDirectory) if @ms_FileDefiningOutputDirectory
		end
		
		if ls_OutputDirectory == nil
			lk_Errors.push("Unable to determine output directory.")
		else
			if (@ms_ScriptType == 'processor')
				@mk_Output.each do |ls_Key, lk_OutputFile|
					ls_FirstUpKey = ls_Key.dup
					ls_FirstUpKey[0, 1] = ls_FirstUpKey[0, 1].upcase
					if @param["[output]write#{ls_FirstUpKey}".intern]
						ls_Path = File.join(ls_OutputDirectory, @param['[output]prefix'.intern] + lk_OutputFile['filename'])
						# ignore prefix if in daemon mode
						ls_Path = File.join(ls_OutputDirectory, lk_OutputFile['filename']) if @mb_Daemon
						@output[ls_Key.intern] = ls_Path
					end
				end
			elsif (@ms_ScriptType == 'converter')
				@mk_Output.keys.each do |ls_OutputGroup|
					@input[ls_OutputGroup.intern].each do |ls_Path|
						ls_Directory = File::dirname(ls_Path)
						ls_Basename = File::basename(ls_Path)
						ls_Format = findFormatForFile(ls_Path)
						ls_Extension = ''
						formatInfo(ls_Format)['extensions'].each do |ls_Try|
							if stringEndsWith(ls_Basename, ls_Try, false)
								ls_Extension = ls_Try
								break
							end
						end
						ls_Basename.slice!(-ls_Extension.size, ls_Extension.size) if ls_Extension != ''
						ls_OutFilename = @mk_Output[ls_OutputGroup]['filename'].gsub('#{basename}', ls_Basename).gsub('#{extension}', ls_Extension)
						ls_OutPath = File::join(ls_OutputDirectory, @param['[output]prefix'.intern] + ls_OutFilename)
						@output[ls_Path] = ls_OutPath
					end
				end
			end
		end
		@outputDirectory = ls_OutputDirectory

		# check if output files already exist
		@output.each_value do |ls_Path|
			lk_Errors.push("#{ls_Path} already exists. I won't overwrite this file.") if File::exists?(ls_Path)
		end
		
		# add .proteomatic.part to each output file
		@output.each_key { |ls_Key| @output[ls_Key] += '.proteomatic.part' }

		raise ProteomaticArgumentException, "Error#{lk_Errors.size > 1 ? "s:\n": ": "}" + lk_Errors.join("\n") unless lk_Errors.empty?
	end
	
	
	def finishOutputFiles()
		lk_Files = Array.new
		@output.each_key do |ls_Key| 
			ls_RealName = @output[ls_Key].sub('.proteomatic.part', '')
			if File::exists?(@output[ls_Key])
				FileUtils::mv(@output[ls_Key], ls_RealName)
				lk_Files.push(File::basename(ls_RealName))
			end
		end
		
		# delete output files
		FileUtils::rm_f(@mk_TempFiles)
		return lk_Files
	end
	
	
	def fileDefiningOutputDirectory()
		return @ms_FileDefiningOutputDirectory
	end


	def findGroupForFile(as_Path)
		@mk_Input['groups'].each do |ls_Group, lk_Group|
			lk_Group['formats'].each do |ls_Format|
				return ls_Group if (fileMatchesFormat(as_Path, ls_Format))
			end
		end
		return nil
	end
	private :findGroupForFile

	def findFormatForFile(as_Path)
		@mk_Input['groups'].each do |ls_Group, lk_Group|
			lk_Group['formats'].each do |ls_Format|
				return ls_Format if (fileMatchesFormat(as_Path, ls_Format))
			end
		end
		return nil
	end
	private :findFormatForFile

	
	def param(as_Key)
    	return @mk_Parameters.value(as_Key)
	end
	
	def inputFiles(as_Group)
		return @mk_Input['files'][as_Group]
	end
	
	def tempFilename(as_Prefix = 'temp-')
		lk_TempFile = Tempfile.new(as_Prefix, @outputDirectory)
		ls_TempFilename = lk_TempFile.path
		lk_TempFile.close!
		@mk_TempFiles.push(ls_TempFilename)
		return ls_TempFilename
	end

	def run()
		raise 'Internal error: run() must be overridden!'
		exit 1
	end
	
	def getConfigValue(as_Key)
		lk_Config = YAML::load_file(File::join('config', "#{@ms_ScriptName}.config.yaml"))
		return lk_Config[as_Key]
	end
end
