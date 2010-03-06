require 'date'

unless ARGV.empty?
    Dir::chdir(ARGV.first)
end

bins = Hash.new
weekBins = Hash.new

(0..6).each do |wday|
    (0..23).each do |hour|
        bins["#{wday}-#{hour}"] = 0
    end
end

currentBin = nil
currentWeekBin = nil

#r = /.*(\d+) insertions.*(\d+) deletions.*/
r = /(\d+) insertions\(\+\), (\d+) deletions/

lines = `git log --stat --summary`
lines.each_line do |line|
    if line[0, 5] == 'Date:'
        d = DateTime.parse(line[5, line.size])
        currentBin = "#{(d.wday + 6) % 7}-#{d.hour}"
        currentWeekBin = d.jd / 7
    elsif line.strip =~ r
        ins = r.match(line.strip)[1].to_i
        del = r.match(line.strip)[2].to_i
        bins[currentBin] += ins
        weekBins[currentWeekBin] ||= 0
        weekBins[currentWeekBin] += ins
    end
end

max = 0
bins.values.each do |x|
    max = x if x > max
end


dot = 32
iw = dot * 24 + 48
ih = dot * 7 + 24
File::open('git-log.svg', 'w') do |f|
    f.puts "<?xml version='1.0' encoding='utf-8' ?>"
    f.puts "<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.1' width='#{iw}' height='#{ih}'>"
    f.puts "<rect width='#{iw}' height='#{ih}' fill='#fff'/>"
    (0..24).each do |h|
        f.puts "<text style='font-family: sans-serif; font-size: 12px;' x='#{h * dot + 8 + 48}' y='#{dot * 7 + 10}'>#{sprintf('%02d', h)}</text>"
    end
    (0..6).each do |wday|
        f.puts "<text style='font-family: sans-serif; font-size: 12px;' x='#{24}' y='#{wday * dot + 20}'>#{Date::ABBR_DAYNAMES[(wday + 1) % 7]}</text>"
    end
    (0..6).each do |wday|
        (0..23).each do |hour|
            r = ((bins["#{wday}-#{hour}"].to_f / max) ** 0.2) * dot / 2.0
            f.puts "<circle cx='#{dot / 2.0 + hour * dot + 48}' cy='#{dot / 2.0 + wday * dot}' r='#{r}' style='fill:#c0c0c0' />"
        end
    end
    f.puts '</svg>'
end

exit
weeksSorted = weekBins.keys.sort
firstWeek = weeksSorted.first
lastWeek = weeksSorted.last
(firstWeek..lastWeek).each { |x| weekBins[x] ||= 0 }

(firstWeek..lastWeek).each do |x|
    puts weekBins[x]
end
