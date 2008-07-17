require 'fileutils'
require 'yaml'
require 'scripts/include/misc'

ls_Version = nil
File.open('scripts/include/version.rb', 'r') { |lk_File| ls_Version = lk_File.read.strip }

if ls_Version == nil
	puts 'Internal error: Unable to determine Proteomatic version.'
	exit 1
end

puts "Creating release for Proteomatic #{ls_Version}..."

ls_Platform = determinePlatform()
ls_Make = {'windows' => 'nmake', 'linux' => 'make', 'mac' => 'make'}
ls_QMake = {'windows' => 'qmake', 'linux' => 'qmake', 'mac' => 'qmake -spec macx-g++'}
ls_BinaryExtension = {'windows' => '.exe', 'linux' => '', 'mac' => ''}

ls_DestDir = "proteomatic-#{ls_Version}-#{ls_Platform}"
FileUtils.rmtree(ls_DestDir)
FileUtils.mkpath(ls_DestDir)
FileUtils.rm_rf(ls_DestDir + '.tar')
FileUtils.rm_rf(ls_DestDir + '.tar.bz2')
FileUtils.rm_rf(ls_DestDir + '.zip')

puts 'Building Proteomatic executables...'

FileUtils.rmtree(File::join('src', 'obj'))
lk_Projects = ['Proteomatic']
lk_Projects.each { |ls_Project| system("cd src/projects/#{ls_Project} && #{ls_QMake[ls_Platform]} && #{ls_Make[ls_Platform]} release && cd ../../../") }

puts 'Collecting Proteomatic executables...'

lk_Projects.each { |ls_Project| FileUtils.cp(ls_Project + ls_BinaryExtension[ls_Platform], ls_DestDir) }

if (ls_Platform == 'windows')
	lk_Projects.each { |ls_Project| FileUtils.cp(ls_Project + '.exe.manifest', ls_DestDir) }
	FileUtils.cp('C:/Qt/4.3.4/bin/QtCore4.dll', ls_DestDir)
	FileUtils.cp('C:/Qt/4.3.4/bin/QtGui4.dll', ls_DestDir)
	FileUtils.cp('C:/Qt/4.3.4/bin/QtNetwork4.dll', ls_DestDir)
	FileUtils.cp('C:/Programme/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT/msvcp90.dll', ls_DestDir)
	FileUtils.cp('C:/Programme/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT/msvcr90.dll', ls_DestDir)
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
	
	system("scripts/include/helpers/7z/7za.exe a -r #{ls_DestDir}.zip #{ls_DestDir}")
	
	puts 'Compiling installer...'
	
	ls_Script = DATA.read()
	ls_Script.gsub!('#{VERSION}', ls_Version)
	ls_Script.gsub!('#{DESTPATH}', ls_DestDir)
	
	lk_File = File.open('temp.nsi', 'w')
	lk_File.write(ls_Script)
	lk_File.close()
	
	system("\"c:/Programme/NSIS/makensis.exe\" temp.nsi")
	
	FileUtils.rm_f(['temp.nsi'])
else
	puts 'Builing bzip2 package...'
	system("tar cvf #{ls_DestDir}.tar #{ls_DestDir}")
	system("bzip2 -z #{ls_DestDir}.tar")
end

FileUtils.rmtree(ls_DestDir)
FileUtils.rmtree(File::join('src', 'obj'))
lk_Projects.each { |ls_Project| FileUtils::rm_rf(ls_Project + ls_BinaryExtension[ls_Platform]) }
lk_Projects.each { |ls_Project| FileUtils.rm_rf(ls_Project + '.exe.manifest') } if (ls_Platform == 'windows')

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
