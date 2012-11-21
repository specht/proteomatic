require 'open-uri'
require 'uri'
require 'net/ftp'
require 'tempfile'
require 'fileutils'
require 'yaml'
require 'digest/md5'

DANGLING_LOCK_KEEP_TIME = 7 * 24 * 60 * 60 # this is seven days in seconds!

$stdout.sync = true
$stderr.sync = true

$scriptsPath = nil

# change into this script's directory
Dir::chdir(File.dirname(__FILE__))

$scriptDir = Dir::pwd()

def determinePlatform()
    case RUBY_PLATFORM.downcase
    when /linux/ then
        'linux'
    when /darwin/ then
        'macx'
    when /mswin/ then
        'win32'
    when /i386-mingw32/
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
    li_BlockSize = 16384 * 8
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
        open(as_Uri, 
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
        command = "\"#{File::join($scriptDir, '7zip', '7za457', '7za.exe')}\" x -bd \"#{path}\""
		output = %x{#{command}}
        FileUtils::rm_f(path)
        command = "\"#{File::join($scriptDir, '7zip', '7za457', '7za.exe')}\" x -bd \"#{path.sub('.bz2', '')}\""
		output = %x{#{command}}
        FileUtils::rm_f(path.sub('.bz2', ''))
		return $?.exitstatus == 0
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
    tempPath = File::join('temp', File::basename(info['platforms'][$platform]['path']))
    fetchUriAsFile(info['platforms'][$platform]['path'], tempPath)
    puts
    print 'Unpacking...'
    Dir::chdir('temp')
    unpack(File::basename(tempPath))
    puts " done."
    Dir::chdir('..')
    FileUtils::rm(tempPath)
    dirs = Dir['temp/*']
    dirs.reject! do |path|
        !File::directory?(path)
    end
    if dirs.size != 1
        puts "Error: The downloaded package was corrupt."
        exit(1)
    end
    newDir = dirs.first + '/'
    print 'Patching files...'
    files = Dir[File::join(newDir, '**', '*')]
    targetDir = File::join('..')
    lockedFiles = Array.new
    files.each do |path|
        sourcePath = path.dup
        sourceDirPrefix = newDir
        sourceDir = File::dirname(sourcePath).sub(newDir, '')
        sourceBasename = File::basename(sourcePath)
        
        next if File::directory?(sourcePath)
        
        # no chance to update Proteomatic wrapper, so skip it
        next if (File::file?(sourcePath) && (sourceBasename == 'Proteomatic' || sourceBasename == 'Proteomatic.exe'))
        
        # ignore scripts
        next if sourceDir.index('scripts/') == 0

        # ignore pax_global_header
        next if sourcePath.include?('pax_global_header')
        
        targetPath = File::join(targetDir, sourceDir)
        FileUtils::mkpath(targetPath) unless File::exists?(targetPath)
        begin
            FileUtils::cp(sourcePath, targetPath)
            # puts "Copying #{sourcePath} to #{targetPath}"
        rescue
            # we could not overwrite the file because it is locked, record it
            FileUtils::mv(sourcePath, sourcePath + '_updated')
            sourcePath += '_updated'
            FileUtils::cp(sourcePath, targetPath)
            # puts "RETRY: Copying #{sourcePath} to #{targetPath}"
            lockedFiles << File::join(sourceDir, File::basename(sourcePath))
        end
    end
    File::open('../update-finish.txt', 'w') do |f|
        lockedFiles.each do |s|
            f.puts s
        end
    end
    puts ' done.'
end


def updateScripts(info)
    FileUtils::rm_rf('temp')
    FileUtils::mkdir('temp')
    # determine currently installed scripts version
    scriptsDir = $scriptsPath
    scriptsDir ||= File::join('..', 'scripts')
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
        tempPath = File::join('temp', File::basename(info['path']))
        fetchUriAsFile(info['path'], tempPath)
        puts
        print "Unpacking..."
        Dir::chdir('temp')
        unpack(File::basename(tempPath))
        Dir::chdir('..')
        # check whether there is a directory
        dirs = Dir['temp/*']
        dirs.reject! do |path|
            !File::directory?(path)
        end
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
                
                # don't delete if it's the fresh, just-fetched-and-unpacked package!
                next if path == newDir
                
                # now see if there are any locks, either a lock is there because another
                # Proteomatic is using it and it will be deleted when this Proteomatic 
                # has finished, OR: the lock is dangling because Proteomatic crashed
                # at some point and the lock is still there. 
                # Solution: Ignore the lock if it's too old already, as defined in
                # DANGLING_LOCK_KEEP_TIME
                lockFiles = Dir[File::join(path, '.lock', '*')]
                lockFiles.reject! { |x| (Time.now - File.mtime(x)) > DANGLING_LOCK_KEEP_TIME }
                next unless lockFiles.empty?
                
                # now delete the script package
                FileUtils::rm_rf(path) 
                
            end
        else
            puts "Error: The downloaded package was corrupt."
            exit(1)
        end
    end
end


ls_Uri = ''

if ARGV.size < 1
    puts 'Usage: ruby update.rb [Proteomatic update URI] [--dryrun] [--scriptsPath <path>] [packages]'
    puts "Packages may be 'scripts' or 'proteomatic'."
    exit 1
end

ls_Uri = ARGV[0]

lb_DryRun = false
if ARGV.include?('--dryrun')
    lb_DryRun = true
    ARGV.delete('--dryrun')
end

if ARGV.include?('--scriptsPath')
    i = ARGV.index('--scriptsPath')
    $scriptsPath = ARGV[i + 1]
    ARGV.delete_at(i)
    ARGV.delete_at(i)
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
        when 'proteomatic' then
            updateProteomatic(lk_CurrentInfo[package])
        when 'scripts' then
            updateScripts(lk_CurrentInfo[package])
        end
    end
end

FileUtils::rm_rf('temp')

puts "Update completed successfully."
