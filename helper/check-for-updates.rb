require 'open-uri'
require 'uri'
require 'net/ftp'
require 'tempfile'
require 'fileutils'
require 'yaml'


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


ls_Uri = ''

if ARGV.size < 1
    puts 'Usage: ruby check-for-updates.rb [proteomatic-scripts URI] [(optional) --dryrun] [--outpath .] [--oldpath .]'
    exit 1
end

ls_Uri = ARGV[0]

lb_DryRun = ARGV.include?('--dryrun')
ls_OutPath = '.'
ls_OldPath = ''

if ARGV.include?('--outpath')
    li_Index = ARGV.index('--outpath')
    ls_OutPath = ARGV[li_Index + 1]
end

if ARGV.include?('--oldpath')
    li_Index = ARGV.index('--oldpath')
    ls_OldPath = ARGV[li_Index + 1]
end

unless (File::exists?(ls_OutPath))
    puts "Error: output path #{ls_OutPath} does not exist."
    exit 1
end

ls_Current = ''

begin
    ls_Current = fetchUriAsString(File::join(ls_Uri, 'current.txt'), false).strip
rescue StandardError => e
    puts "Error: Unable to determine current version from #{ls_Uri}"
    exit 1
end

if lb_DryRun
    puts "CURRENT-VERSION:#{ls_Current}"
    exit 0
end

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
