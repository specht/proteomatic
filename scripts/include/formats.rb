require 'yaml'

$gk_FormatCache = Hash.new

def formatInfo(as_Format)
	ls_FormatFile = "include/formats/#{as_Format}.yaml"
	if (!File.exists?(ls_FormatFile))
		puts "Internal error: Could not find format file for #{as_Format}"
		exit 1
	end
	lk_Format = nil
	if $gk_FormatCache.has_key?(as_Format)
		lk_Format = $gk_FormatCache[as_Format]
	else
		lk_Format = YAML::load_file(ls_FormatFile)
		lk_Format['extensions'].collect! { |x| x.downcase }
		$gk_FormatCache[as_Format] = lk_Format
	end
	return lk_Format
end


def fileMatchesFormat(as_Filename, as_Format)
	lk_Format = formatInfo(as_Format)
	# match file extension
	lb_ExtensionMatches = false
	lk_Format['extensions'].each do |ls_Extension|
		if (as_Filename.rindex(ls_Extension) == as_Filename.size - ls_Extension.size)
			lb_ExtensionMatches = true
			break
		end
	end
	return false unless lb_ExtensionMatches
	# try detection by reading some bytes
	eval(lk_Format['detection']) rescue return false
	# if we came through this, throw in a return true 'for good measure'
	return true
end


def assertFormat(as_Format)
	formatInfo(as_Format)
end
