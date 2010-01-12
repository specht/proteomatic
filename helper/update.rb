require 'open-uri'
require 'uri'
require 'net/ftp'
require 'tempfile'
require 'fileutils'
require 'yaml'
require 'digest/md5'

# change into this script's directory
Dir::chdir(File.dirname(__FILE__))

def determinePlatform()
    case RUBY_PLATFORM.downcase
    when /linux/
        'linux'
    when /darwin/
        'macx'
    when /mswin/
        'win32'
    else
        puts "Internal error: #{RUBY_PLATFORM} platform not supported."
        exit 1
    end
end


$platform = determinePlatform()


def bytesToString(ai_Size)
    if ai_Size < 1024
        return "#{ai_Size} bytes"
    elsif ai_Size < 1024 * 1024
        return "#{sprintf('%1.2f', ai_Size.to_f / 1024.0)} KB"
    elsif ai_Size < 1024 * 1024 * 1024
        return "#{sprintf('%1.2f', ai_Size.to_f / 1024.0 / 1024.0)} MB"
    end
    return "#{sprintf('%1.2f', ai_Size.to_f / 1024.0 / 1024.0 / 1024.0)} GB"
end


def fetchUriAsFile(as_Uri, as_Path, ab_ShowProgress = true)
    lk_Uri = URI::parse(as_Uri)
    li_BlockSize = 16384
    ls_Result = ''
    if (as_Uri[0, 6] == 'ftp://')
        Net::FTP.open(lk_Uri.host) do |lk_Ftp|
            lk_Ftp.passive = true 
            lk_Ftp.login
            li_Size = lk_Ftp.size(lk_Uri.path)
            li_Received = 0
            lk_Ftp.getbinaryfile(lk_Uri.path, as_Path, li_BlockSize) do |lk_Data|
                li_Received += lk_Data.size
                print "\rDownloaded #{bytesToString(li_Received)} of #{bytesToString(li_Size)}    " if ab_ShowProgress
            end
        end
    else
        li_Size = 0
        open(ls_Uri, 
            :content_length_proc => lambda { |t| li_Size = t }, 
            :progress_proc => lambda { |t| print "\rDownloaded #{bytesToString(t)} of #{bytesToString(li_Size)}    "  if ab_ShowProgress }) do |lk_RemoteFile|
            File::open(as_Path, 'wb') { |lk_Out| lk_Out.write(lk_RemoteFile.read) }
        end
    end
    return ls_Result
end


def fetchUriAsString(as_Uri, ab_ShowProgress = true)
    lk_TempFile = Tempfile.new('check-for-updates-')
    fetchUriAsFile(as_Uri, lk_TempFile.path(), ab_ShowProgress)
    ls_Result = File::read(lk_TempFile.path())
    lk_TempFile.unlink
    return ls_Result
end


def unpack(path)
    if ($platform == 'win32')
        puts "IMPLEMENT ME!"
        exit 1
    else
        system("tar xjf \"#{path}\"")
    end
end


def updateProteomatic(info)
    unless info['platforms'][$platform]
        puts "Error: No update available for this platform (#{$platform})."
        exit 1
    end
    FileUtils::rm_rf('temp')
    FileUtils::mkdir('temp')
    puts "Fetching Proteomatic #{info['version']}..."
    lk_TempFile = Tempfile.new('p-', 'temp')
    tempPath = lk_TempFile.path()
    fetchUriAsFile(info['platforms'][$platform]['path'], tempPath)
    puts
    print 'Unpacking...'
    Dir::chdir('temp')
    unpack(File::basename(tempPath))
    Dir::chdir('..')
    lk_TempFile.close!
    puts
    dirs = Dir['temp/*']
    if dirs.size != 1
        puts "Error: The downloaded package was corrupt."
        exit(1)
    end
    newDir = dirs.first
    files = Dir[File::join(newDir, '**', '*')]
    targetDir = File::join('..')
    files.each do |path|
        sourcePath = path.dup
        sourceDirPrefix = newDir
        sourceDir = File::dirname(sourcePath).sub(newDir, '')
        sourceBasename = File::basename(sourcePath)
        
        next if File::directory?(sourcePath)
        
        # no chance to update Proteomatic wrapper, so skip it
        next if (sourceDir == '' && (sourceBasename == 'Proteomatic' || sourceBasename == 'Proteomatic.exe'))
        
        # ignore scripts
        next if sourceDir.index('/scripts/') == 0
        
        if sourceBasename == 'ProteomaticCore'
            FileUtils::mv(sourcePath, sourcePath + '_updated')
            sourceBasename += '_updated'
            sourcePath += '_updated'
        end
        
        targetPath = File::join(targetDir, sourceDir)
        FileUtils::mkpath(targetPath) unless File::exists?(targetPath)
        FileUtils::cp(sourcePath, targetPath)
    end
end


def updateScripts(info)
    FileUtils::rm_rf('temp')
    FileUtils::mkdir('temp')
    # determine currently installed scripts version
    scriptsDir = File::join('..', 'scripts')
    unless File::directory?(scriptsDir)
        if File::file?(scriptsDir)
            puts "Error: A file called 'scripts' exists, but we need to create a directory there."
            puts "Please move the file out of the way and try again."
            exit 1
        end
        FileUtils::mkpath(scriptsDir) 
    end
    mostRecentVersion = nil
    availableVersions = Array.new
    Dir[File::join(scriptsDir, 'proteomatic-scripts*')].each do |path|
        next if path.include?('.part')
        package = File::basename(path).sub('proteomatic-scripts-', '')
        availableVersions << package
    end
    availableVersions.uniq!
    availableVersions.sort! do |a, b|
        al = a.split('.')
        bl = b.split('.')
        ai = 0
        bi = 0
        result = nil
        while result == nil
            if (ai == bi)
                if al[ai] != bl[bi]
                    result = (al[ai].to_i <=> bl[bi].to_i)
                    break
                else
                    ai += 1
                    ai = al.size - 1 if ai >= al.size
                    bi += 1
                    bi = bl.size - 1 if bi >= bl.size
                end
            else
                result = ai <=> bi
                break
            end
        end
        result
    end
    mostRecentVersion = availableVersions.last
    if mostRecentVersion == info['version']
        puts "Proteomatic scripts is up-to-date (version #{mostRecentVersion})."
    else
        puts "Fetching Proteomatic scripts #{info['version']}..."
        lk_TempFile = Tempfile.new('ps-', 'temp')
        tempPath = lk_TempFile.path()
        fetchUriAsFile(info['path'], tempPath)
        puts
        print "Unpacking..."
        Dir::chdir('temp')
        unpack(File::basename(tempPath))
        Dir::chdir('..')
        lk_TempFile.close!
        # check whether there is a directory
        dirs = Dir['temp/*']
        if dirs.size == 1
            # maybe insert consistency check here
            FileUtils::mv(dirs.first, scriptsDir)
            puts
            # copy configuration files
            if mostRecentVersion
                print 'Copying configuration files...'
                configFiles = Dir[File::join(scriptsDir, 'proteomatic-scripts-' + mostRecentVersion, 'config', '*')]
                destDir = File::join(scriptsDir, 'proteomatic-scripts-' + info['version'], 'config')
                configFiles.each do |path|
                    # copy config file unless it already exists in the fresh package
                    unless File::exists?(File::join(destDir, File::basename(path)))
                        FileUtils::cp(path, destDir)
                    end
                end
                puts
            end
            # remove old script packages
            newDir = File::join(scriptsDir, 'proteomatic-scripts-' + info['version'])
            Dir[File::join(scriptsDir, '*')].each do |path|
                FileUtils::rm_rf(path) unless path == newDir
            end
        else
            puts "Error: The downloaded package was corrupt."
            exit(1)
        end
    end
end


ls_Uri = ''

if ARGV.size < 1
    puts 'Usage: ruby update.rb [Proteomatic update URI] [(optional) --dryrun] [packages]'
    puts "Packages may be 'scripts' or 'proteomatic'."
    exit 1
end

ls_Uri = ARGV[0]

lb_DryRun = false
if ARGV.include?('--dryrun')
    lb_DryRun = true
    ARGV.delete('--dryrun')
end

updatePackages = Array.new

ARGV[1, ARGV.size - 1].each do |p|
    updatePackages << p
end

lk_CurrentInfo = nil

begin
    lk_CurrentInfo = YAML::load(fetchUriAsString(File::join(ls_Uri, 'current.yaml'), false))
rescue StandardError => e
    puts "Error: Unable to fetch update information from #{ls_Uri}."
    exit 1
end

if lb_DryRun
    puts "CURRENT-VERSIONS"
    lk_CurrentInfo.each_key do |package|
        puts "#{package}: #{lk_CurrentInfo[package]['version']}"
    end
    exit 0
end

# sort so that proteomatic update comes first
updatePackages.sort!

updatePackages.each do |package|
    if lk_CurrentInfo[package]
        case package
        when 'proteomatic':
            updateProteomatic(lk_CurrentInfo[package])
        when 'scripts':
            updateScripts(lk_CurrentInfo[package])
        end
    end
end

FileUtils::rm_rf('temp')

__END__

ls_PackagePath = File::join(ls_OutPath, ls_Current)
puts "Fetching #{ls_Current}"
$stdout.flush
fetchUriAsFile(File::join(ls_Uri, ls_Current), ls_PackagePath)
puts
$stdout.flush

puts "Unpacking..."
$stdout.flush

ls_OldDir = Dir::pwd()
Dir::chdir(File::dirname(ls_PackagePath))
ls_Platform = determinePlatform()

if (ls_Platform == 'linux' || ls_Platform == 'macx')
    unless system("bzip2 -dc \"#{File::basename(ls_PackagePath)}\" | tar xf -")
        puts "Error: Unable to unpack #{ls_PackagePath}."
        exit 1
    end
elsif (ls_Platform == 'win32')
    ls_Command = "#{File::join(ls_OldDir, '7zip', '7za457', '7za.exe')} x \"#{ls_PackagePath}\""
    begin
        lk_Process = IO.popen(ls_Command)
        lk_Process.read
    rescue StandardError => e
        puts 'Error: There was an error while executing 7zip.'
        exit 1
    end

    FileUtils::rm_f(ls_PackagePath)
    ls_PackagePath.sub!('.bz2', '')
    
    ls_Command = "#{File::join(ls_OldDir, '7zip', '7za457', '7za.exe')} x \"#{ls_PackagePath}\""
    begin
        lk_Process = IO.popen(ls_Command)
        lk_Process.read
    rescue StandardError => e
        puts 'Error: There was an error while executing 7zip.'
        exit 1
    end
end

$stdout.flush
unless (ls_OldPath.empty?)
    puts "Copying configuration files..."
    $stdout.flush
    lk_OldFiles = Dir[File::join(ls_OldPath, 'config/**/*')]
    #lk_OldFiles += Dir[File::join(ls_OldPath, 'ext/**/*')]
    lk_OldFiles.collect! { |x| x.sub(ls_OldPath, '') }
    ls_NewPath = File::join(ls_OutPath, ls_Current.sub('.tar.bz2', ''))
    lk_NewFiles = Dir[File::join(ls_NewPath, 'config/**/*')]
    #lk_NewFiles += Dir[File::join(ls_NewPath, 'ext/**/*')]
    
    # strip base dir
    lk_NewFiles.collect! { |x| x.sub(ls_NewPath, '') }
    
    # reject files that are already in the new location
    lk_OldFiles.reject! { |x| lk_NewFiles.include?(x) }
    
    # expand to full path again
    lk_OldFiles.collect! { |x| File::join(ls_OldPath, x) }
    
    # extact dirs
    lk_OldExtraDirs = lk_OldFiles.select { |x| File::directory?(x) } 
    
    # reject dirs
    lk_OldFiles.reject! { |x| File::directory?(x) }
    
    # create dirs in new location
    lk_OldExtraDirs.each do |ls_Dir|
        FileUtils::mkpath(File::join(ls_NewPath, ls_Dir.sub(ls_OldPath, '')))
    end
    
    # copy files to new location
    lk_OldFiles.each do |ls_File|
        FileUtils::cp(ls_File, File::join(ls_NewPath, ls_File.sub(ls_OldPath, '')))
    end
end

puts "Update completed successfully."

Dir::chdir(ls_OldDir)
FileUtils::rm_f(ls_PackagePath)
