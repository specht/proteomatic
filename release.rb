require 'fileutils'
require 'yaml'
require 'scripts/include/misc'

ls_Platform = determinePlatform()

ls_Config = ''

if ls_Platform == 'windows' 
	unless File::exists?('release.conf')
		puts 'Using default config. Please adjust values in release.conf if neccessary.'
		File::open('release.conf', 'w') do |lk_File|
			lk_File.puts "QT_PATH = 'c:/Qt/4.4.1'"
			lk_File.puts "MINGW_PATH = 'c:/MinGW'"
			lk_File.puts "NSIS_PATH = File::join(ENV['ProgramFiles'], 'NSIS')"
		end
	end
	ls_Config = File::read('release.conf')
end

eval(ls_Config)

MINGW_PATH = '' unless defined?(MINGW_PATH)
QT_PATH = '' unless defined?(QT_PATH)
NSIS_PATH = '' unless defined?(NSIS_PATH)

# test config
if ls_Platform == 'windows' 
	lk_Errors = Array.new
	lk_Errors.push("Unable to find MinGW in #{MINGW_PATH}.") unless File::exists?(File::join(MINGW_PATH, 'bin/mingwm10.dll'))
	lk_Errors.push("Unable to find Qt in #{QT_PATH}.") unless File::exists?(File::join(QT_PATH, 'bin/QtCore4.dll'))
	lk_Errors.push("Unable to find NSIS in #{NSIS_PATH}.") unless File::exists?(File::join(NSIS_PATH, 'makensis.exe'))
	unless lk_Errors.empty?
		puts 'Errors:'
		puts lk_Errors.join("\n")
		exit 1
	end
end

ls_Version = File::basename(Dir::pwd())

if ls_Version == nil
	puts 'Internal error: Unable to determine Proteomatic version.'
	exit 1
end

File.open('scripts/include/version.rb', 'w') { |lk_File| lk_File.write(ls_Version) }

puts "Creating release for Proteomatic #{ls_Version}..."

ls_Make = {'windows' => File::join(MINGW_PATH, 'bin', 'mingw32-make.exe'), 'linux' => 'make', 'mac' => 'make'}
ls_QMake = {'windows' => File::join(QT_PATH, 'bin', 'qmake.exe') + ' -spec win32-g++', 'linux' => 'qmake', 'mac' => 'qmake -spec macx-g++'}
ls_BinaryExtension = {'windows' => '.exe', 'linux' => '', 'mac' => ''}

ls_DestDir = "proteomatic-#{ls_Version}-#{ls_Platform}"
FileUtils.rmtree(ls_DestDir)
FileUtils.mkpath(ls_DestDir)
FileUtils.rm_rf(ls_DestDir + '.tar')
FileUtils.rm_rf(ls_DestDir + '.tar.bz2')
FileUtils.rm_rf(ls_DestDir + '.zip')

puts 'Building Proteomatic executables...'

FileUtils.rmtree(File::join('src', 'obj'))
system("cd src/src/libyaml && #{ls_QMake[ls_Platform]} && #{ls_Make[ls_Platform]} release")
lk_Projects = ['Proteomatic']
lk_Projects.each do |ls_Project| 
	unless system("cd src/projects/#{ls_Project} && #{ls_QMake[ls_Platform]} && #{ls_Make[ls_Platform]} release && cd ../../../")
		puts 'There was an error!'
		exit 1
	end
end

puts 'Collecting Proteomatic executables...'

lk_Projects.each { |ls_Project| FileUtils.cp(ls_Project + ls_BinaryExtension[ls_Platform], ls_DestDir) }

if (ls_Platform == 'windows')
	FileUtils.cp(File::join(QT_PATH, 'bin/QtCore4.dll'), ls_DestDir)
	FileUtils.cp(File::join(QT_PATH, 'bin/QtGui4.dll'), ls_DestDir)
	FileUtils.cp(File::join(QT_PATH, 'bin/QtNetwork4.dll'), ls_DestDir)
	FileUtils.cp(File::join(MINGW_PATH, 'bin/mingwm10.dll'), ls_DestDir)
end

puts 'Collecting scripts...'
lk_Files = Dir.glob(File.join('scripts', '*'))
lk_Files += Dir.glob(File.join('scripts', 'include', '**', '*'))
lk_Files += Dir.glob(File.join('scripts', 'config', '*.template.yaml'))
lk_Files.each { |ls_Path| FileUtils.mkpath(File::join(ls_DestDir, ls_Path)) if File.directory?(ls_Path) }
lk_Files.delete_if { |ls_Path| File.directory?(ls_Path) || ls_Path.include?('.defunct.') }
lk_Files.each do |ls_Path|
	#puts ls_Path
	FileUtils.cp(ls_Path, File::join(ls_DestDir, File.dirname(ls_Path), File.basename(ls_Path)))
end

if (ls_Platform == 'windows')
	puts 'Building ZIP package...'
	
	unless system("scripts/include/helpers/7z/7za.exe a -r #{ls_DestDir}.zip #{ls_DestDir}")
		puts 'There was an error!'
		exit 1
	end
	
	puts 'Compiling installer...'
	
	ls_Script = DATA.read()
	ls_Script.gsub!('#{VERSION}', ls_Version)
	ls_Script.gsub!('#{DESTPATH}', ls_DestDir)
	
	lk_File = File.open('temp.nsi', 'w')
	lk_File.write(ls_Script)
	lk_File.close()
	
	unless system("\"#{File::join(NSIS_PATH, 'makensis.exe')}\" temp.nsi")
		puts 'There was an error!'
		exit 1
	end
	
	FileUtils.rm_f(['temp.nsi'])
else
	puts 'Builing bzip2 package...'
	unless system("tar cvf #{ls_DestDir}.tar #{ls_DestDir}")
		puts 'There was an error!'
		exit 1
	end
	unless system("bzip2 -z #{ls_DestDir}.tar")
		puts 'There was an error!'
		exit 1
	end
end
 
FileUtils.rmtree(ls_DestDir)
FileUtils.rmtree(File::join('src', 'obj'))
lk_Projects.each { |ls_Project| FileUtils::rm_rf(ls_Project + ls_BinaryExtension[ls_Platform]) }

__END__
;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Proteomatic"
  OutFile "Proteomatic #{VERSION} Setup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Proteomatic"
  
  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  ;!insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Proteomatic" SecMain

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
  File /r "#{DESTPATH}\*.*"
  
  CreateDirectory "$SMPROGRAMS\Proteomatic"
  SetOutPath "$INSTDIR"
  createShortCut "$SMPROGRAMS\Proteomatic\Proteomatic.lnk" "$INSTDIR\Proteomatic.exe"
  SetOutPath "$INSTDIR"
  createShortCut "$SMPROGRAMS\Proteomatic\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Proteomatic" \
                   "DisplayName" "Proteomatic - A rubylicous Proteomics Pipeline"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Proteomatic" \
                   "UninstallString" "$INSTDIR\Uninstall.exe"
SectionEnd

;--------------------------------
;Descriptions

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "Proteomatic"
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"
  ;RMDir /r "$SMPROGRAMS\Proteomatic"

  ;DeleteRegKey /ifempty HKCU "Software\Proteomics Pipeline"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Proteomatic"

SectionEnd
