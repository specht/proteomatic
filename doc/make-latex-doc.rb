require 'yaml'
require 'fileutils'
#require 'RedCloth'
#require 'RMagick'

ls_Guide = File::read('proteomatic.yaml')
lk_Guide = YAML::load(ls_Guide)

FileUtils::cp('doc-head.tex', 'Proteomatic.tex')

$gi_ImageCount = 0

def handleString(as_Text, ak_Out)
	ls_Text = as_Text.rstrip
	if ls_Text.include?("\n")
		handleSection(ls_Text, ak_Out, ai_Level + 1)
		next
	end
	puts ls_Text
	if (ls_Text[0, 3] == 'h3.')
		ls_Text.sub!('h3.', '')
		ls_Text.strip!
		ak_Out << "\n\n\\subsection{#{ls_Text}}\n"
	elsif ls_Text =~ /\A![<>]?.+/
		puts 'image!'
		puts ls_Text
		ls_Name = ls_Text
		ls_Direction = ls_Text[1, 1]
		ls_Direction = '' if ls_Direction !~ /[<>]/
		ls_Name.gsub!(/\A![<>]?/, '')
		ls_Name.gsub!(/\(.*\)/, '')
		ls_Name.gsub!('!', '')
		ls_Name.strip!
		lf_Width = 1.0
		if (ls_Direction.empty?)
			ak_Out << "	\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Name}} \n"
		else
			ak_Out << "\\begin{wrapfigure}{#{ls_Direction == '<' ? 'l' : 'r'}}[0cm]{#{lf_Width}\\textwidth}\n"
			ak_Out << "  \\begin{center}\n"
			ak_Out << "	    \\includegraphics[width=#{lf_Width * 0.95}\\textwidth, height=#{lf_Height * 0.95}\\textwidth]{#{ls_Name}}\n"
			ak_Out << "  \\end{center}\n"
			ak_Out << "\\end{wrapfigure}\n"
		end
	elsif ls_Text[0, 4] == '<pre'
		ak_Out << "\\begin{lstlisting}\n"
		lb_InPre = true
	elsif ls_Text[0, 5] == '</pre'
		ak_Out << "\\end{lstlisting}\n"
		lb_InPre = false
	elsif !ls_Text.include?('#') && !ls_Text.include?('&') && !ls_Text.include?('|') && !ls_Text.include?('$')
		unless lb_InPre
			li_StartIndex = 0
			while ls_Text.index(/(\s|\A|\W)@(\S)/, li_StartIndex) != nil
				li_Index = ls_Text.index(/(\s|\A|\W)@(\S)/, li_StartIndex)
				while ls_Text[li_Index + 2, 1] == '@'
					li_Index += 1
				end
				ls_Text[ls_Text.index(/(\s|\A|\W)@(\S)/, li_StartIndex) + 1, 1] = 'CODE-START'
				li_EndIndex = ls_Text.index('@', li_Index + 11)
				ls_Text[li_EndIndex, 1] = 'CODE-END'
				ls_Text[li_Index..li_EndIndex] = ls_Text[li_Index..li_EndIndex].gsub('*', '[MICHA:STAR]')
				ls_Text[li_Index..li_EndIndex] = ls_Text[li_Index..li_EndIndex].gsub('_', '[MICHA:UNDERSCORE]')
				ls_Text[li_Index..li_EndIndex] = ls_Text[li_Index..li_EndIndex].gsub('"', '[MICHA:QUOTE]')
				li_StartIndex = li_EndIndex
			end
			lk_Items = ls_Text.split('CODE-START')
			lk_Items.each_index do |i|
				ls_Item = lk_Items[i]
				li_Index = ls_Item.index('CODE-END')
				ls_CodeText = ''
				ls_PlainText = ls_Item
				if (li_Index != nil)
					ls_CodeText = ls_Item[0, li_Index]
					ls_PlainText = ls_Item[li_Index, ls_Item.size - li_Index]
				end
				while (ls_PlainText =~ /(\s|\A|\W)_(\S)/)
					ls_PlainText.sub!(/(\s|\A|\W)_(\S)/, '\\1{\em \\2')
					ls_PlainText.sub!(/(\S)_(\s|\z|\W)/, '\\1}\\2')
				end
				ls_PlainText.gsub!('_', '\_')
				while (ls_PlainText =~ /(\s|\A|\W)\*(\S)/)
					ls_PlainText.sub!(/(\s|\A|\W)\*(\S)/, '\\1{\bf \\2')
					ls_PlainText.sub!(/(\S)\*(\s|\z|\W)/, '\\1}\\2')
				end
				while (ls_PlainText =~ /(\s|\A|\W)"(\S)/)
					ls_PlainText.sub!(/(\s|\A|\W)"(\S)/, '\\1``\\2')
					ls_PlainText.sub!(/(\S)"(\s|\z|\W)/, '\\1\'\'\\2')
				end
				lk_Items[i] = ''
				lk_Items[i] += 'CODE-START' + ls_CodeText unless ls_CodeText.empty?
				lk_Items[i] += ls_PlainText
			end
			ls_Text = lk_Items.join('')
			while (ls_Text.include?('CODE-START'))
				ls_Code = ls_Text[ls_Text.index('CODE-START')..ls_Text.index('CODE-END')]
				lc_Char = '^'
				lc_Char = '$' if (ls_Code.include?(lc_Char))
				ls_Text.sub!('CODE-START', '\lstinline' + lc_Char)
				ls_Text.sub!('CODE-END', lc_Char)
			end
		end
		lb_BlockQuote = ls_Text[0, 3] == 'bq.'
		if lb_BlockQuote
			ak_Out << "\\begin{quote}\n"
			ls_Text.sub!('bq.', '')
			ls_Text.strip!
		end
		ls_Text.gsub!('[MICHA:STAR]', '*')
		ls_Text.gsub!('[MICHA:UNDERSCORE]', '_')
		ls_Text.gsub!('[MICHA:QUOTE]', '"')
		ak_Out << ls_Text << "\n"
		ak_Out << "\\end{quote}\n" if lb_BlockQuote
	else
		#puts ls_Text
	end
=begin			
	while (ls_Text.include?(' @'))
		li_Start = ls_Text.index(' @')
		li_Start += 1
		li_First = li_Start
		while (ls_Text[li_Start, 1] == '@')
			li_Start += 1
		end
		li_End = ls_Text.index('@', li_Start)
		lc_Char = '^'
		lc_Char = '$' if (ls_Text[li_First..li_End].include?(lc_Char))
		ls_Text[li_End, 1] = lc_Char
		ls_Text[li_First, 1] = '\lstinline' + lc_Char
		ls_Text[li_First...(li_End + 10)] = ls_Text[li_First...(li_End + 10)].gsub('\_', '_')
	end
	while (ls_Text.include?(' *'))
		ls_Text.sub!(' *', ' {\bf ')
		ls_Text.sub!('*', '}')
	end
	if (ls_Text[0, 2] == '# ')
		ls_Text.sub!('# ', "\\item")
		ls_Text = "\\begin{enumerate}\n " + ls_Text if (!lb_InEnumeration)
		lb_InEnumeration = true
	elsif (lb_InEnumeration && ls_Text[0, 2] != '# ')
		lb_InEnumeration = false
		ls_Text = "\\end{enumerate}" + ls_Text
	end
	
	if (ls_Text[0, 4] == 'bq. ')
		ls_Text.sub!('bq. ', "\\begin{quote}\n")
		ls_Text << "\\end{quote}\n"
	end

	if (ls_Text[0, 4] == 'h3. ')
		ls_Text.sub!('h3. ', "\\subsection{")
		ls_Text << "}\n"
	end

	ls_Text.gsub!(/<pre.*>/, '\begin{lstlisting}')
	ls_Text.gsub!('</pre>', '\end{lstlisting}')
	ls_Text.gsub!('<p>', "\n\n")
	ls_Text.gsub!('</p>', "\n\n")
=end			
=begin			
	while (ls_Text.include?('!i/'))
		li_Start = ls_Text.index('!i/')
		li_End = ls_Text.length
		ls_Name = ls_Text[li_Start..li_End]
		ls_Name.gsub!(/\(.*\)/, '')
		ls_Name.gsub!('!', '')
		ls_Name.strip!
		ls_Name = 'wpgtr/' + ls_Name + '.png'
		FileUtils::cp(ls_Name, File::dirname(ls_Name) + $gi_ImageCount.to_s + '.png')
		ls_Name = File::dirname(ls_Name) + $gi_ImageCount.to_s + '.png'
		$gi_ImageCount += 1
		ls_Text[li_Start..li_End] = 
"\\begin{figure}[h] \
\\centering \
\\begin{minipage}[c]{1.0\\textwidth} \
\\includegraphics[width=\\textwidth]{#{ls_Name}} \
\\end{minipage} \
\\end{figure}"
	end
=end			
	
=begin			
	while (ls_Text.include?('bq.'))
		ls_Text.sub!(/bq.*$/, '')
	end
	ak_Out << ls_Text << "\n"
=end
end

=begin
scope stack
- try to close current scope
- no multi scopes

=end

$gk_Rules = Hash.new
$gk_Rules['italic'] = {:char => '_', :start => '{\em ', :end => '}'}
$gk_Rules['bold'] = {:char => '*', :start => '{\bf ', :end => '}'}
$gk_Rules['quote'] = {:char => '"', :start => '``', :end => "''"}
$gk_Rules['code'] = {:char => '@', :start => '\lstinline[breaklines=true]§', :end => '§'}

$gk_Substitution = Array.new
$gk_Substitution.push({:char => '<p>', :replace => "\n\n"})
$gk_Substitution.push({:char => '</p>', :replace => "\n\n"})
$gk_Substitution.push({:char => '#', :replace => "\\#"})
$gk_Substitution.push({:char => '_', :replace => '\_'})
$gk_Substitution.push({:char => '&', :replace => '\&'})
$gk_Substitution.push({:char => '$', :replace => '\$'})
$gk_Substitution.push({:char => " '", :replace => ' `'})

$gk_BlockScopes = Hash.new
$gk_BlockScopes['bq'] = {:char => 'bq. ', :start => "\\begin{quote}\n", :end => "\\end{quote}\n"}
$gk_BlockScopes['h3'] = {:char => 'h3. ', :start => "\n\n\\subsection{", :end => "}\n\n"}

$gb_List = false
$gb_Table = false
$gb_Code = false

def handleString(as_Text, ak_Out)
	as_Text.rstrip!
	if (as_Text.include?("\n"))
		as_Text.each do |s|
			handleString(s, ak_Out)
		end
		return
	end
	
	# handle images
	if as_Text =~ /\A![<>_\*]?.+/
		ls_Name = as_Text
		ls_Direction = as_Text[1, 1]
		ls_Direction = '' if ls_Direction !~ /[<>_\*]/
		ls_Name.gsub!(/\A![<>_\*]?/, '')
		ls_Name.gsub!(/\(.*\)/, '')
		ls_Name.gsub!('!', '')
		ls_Name.strip!
		lf_Width = 1.0
		lf_Height = 1.0
		if (ls_Name.include?('['))
			lk_Name = ls_Name.split('[')
			lf_Width = lk_Name[1]
			ls_Name = lk_Name[0]
			lf_Width.sub!(']', '')
			lf_Width = lf_Width.to_f
		end
		ls_Destination = ls_Name
		IO.popen("identify #{ls_Name}") do |f|
			ls_Result = f.gets
			lk_Size = ls_Result.split(' ')[2].split('x')
			li_Width = lk_Size[0].to_i
			li_Height = lk_Size[1].to_i
			lf_Height = lf_Width / li_Width * li_Height
		end
		if (ls_Direction.empty?)
			ak_Out << "	\\image{label}{#{ls_Destination}}{#{lf_Width}}{}{}\n"
		else
			if (ls_Direction == '_')
				ak_Out << "\\vfill\n\n"
				ak_Out << "\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Destination}} \n\n"
				ak_Out << "\\clearpage\n\n"
			elsif ls_Direction == '*'
				ak_Out << "\\vspace*{0.6cm}\n\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Destination}} \n\\newpage\n"
			else
				ak_Out << "	\\parpic[#{ls_Direction == '<' ? 'l' : 'r'}]{\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Destination}}} \n"
			end
		end
		return
	end
	
	# handle paragraphs
	lk_BlockScopes = Array.new
	unless $gb_Code
		if ($gb_List && as_Text.lstrip[0, 1] != '#')
			ak_Out << "\\end{enumerate}\n"
			$gb_List = false
		end
		if (as_Text.lstrip[0, 1] == '#' && !$gb_List)
			ak_Out << "\\begin{enumerate}\n"
			$gb_List = true
		end
		as_Text.sub!('#', '\item') if (as_Text.lstrip[0, 1] == '#')
		if ($gb_Table && as_Text.lstrip[0, 1] != '|')
			ak_Out << "\\end{tabular}\n"
			$gb_Table = false
		end
		if (!$gb_Table && as_Text.lstrip[0, 1] == '|')
			ak_Out << "\\begin{tabular}{p{0.075\\textwidth}p{0.4\\textwidth}p{0.44\\textwidth}}\n"
			$gb_Table = true
		end
		if $gb_Table
			as_Text.strip!
			as_Text = as_Text[1, as_Text.size - 2]
			if (as_Text.include?('\3.'))
				as_Text.sub!('\3.', '')
				as_Text.gsub!('*', '')
				ak_Out << "\\multicolumn{3}{l}{\\bf #{as_Text}} \\\\\n"
				return
			end
		end
	end
	$gk_BlockScopes.each do |ls_Key, lk_Scope|
		if (as_Text[0, lk_Scope[:char].size] == lk_Scope[:char])
			lk_BlockScopes.push(ls_Key)
			ak_Out << lk_Scope[:start]
			as_Text.gsub!(lk_Scope[:char], '')
		end
	end
	lk_Scopes = Array.new
	i = -1
	while true
		i += 1
		break if i >= as_Text.size
		# try to close scope
		lb_Next = false
		unless lk_Scopes.empty?
			if as_Text[i, $gk_Rules[lk_Scopes.last][:char].size] == $gk_Rules[lk_Scopes.last][:char]
				if i == as_Text.size - 1 || as_Text[i + 1, 1] =~ /[\s\)\.\,:"\?!_\*@']/
					unless as_Text[i - 1, 1] == '@' && lk_Scopes.last == 'code'
						ak_Out << $gk_Rules[lk_Scopes.last][:end]
						$gb_Code = false if (lk_Scopes.last == 'code')
						lk_Scopes.pop
						lb_Next = true
					end
				end
			end
		end
		next if lb_Next
		# try to open a new scope
		$gk_Rules.each do |ls_Key, lk_Rule|
			next if lb_Next
			next if lk_Scopes.include?(ls_Key)
			next if $gb_Code
			if as_Text[i, lk_Rule[:char].size] == lk_Rule[:char]
			next unless i == 0 || as_Text[i - 1, 1] =~ /[\s\("']/
				ak_Out << lk_Rule[:start]
				lk_Scopes.push(ls_Key)
				lb_Next = true
				$gb_Code = true if ls_Key == 'code'
			end
		end
		next if lb_Next
		# try to substitute
		$gk_Substitution.each do |lk_Rule|
			next if lb_Next
			next if $gb_Code
			if as_Text[i, lk_Rule[:char].size] == lk_Rule[:char]
				ak_Out << lk_Rule[:replace]
				i += lk_Rule[:char].size - 1
				lb_Next = true
			end
		end
		next if lb_Next
		if (as_Text[0, 18] == '<pre class="text">')
			i += 17
			ak_Out << "\n\\begin{lstlisting}\n"
			$gb_Code = true
			next
		end
		if (as_Text[0, 20] == '<pre class="result">')
			i += 19
			ak_Out << "\n\\begin{lstlisting}\n"
			$gb_Code = true
			next
		end
		if (as_Text[0, 5] == '<pre>')
			i += 4
			ak_Out << "\n\\begin{lstlisting}\n"
			$gb_Code = true
			next
		end
		if (as_Text[0, 6] == '</pre>')
			i += 5
			ak_Out << "\n\\end{lstlisting}\n"
			$gb_Code = false
			next
		end	
		if (as_Text[0, 7] == ' </pre>')
			i += 6
			ak_Out << "\n\\end{lstlisting}\n"
			$gb_Code = false
			next
		end	
		if !$gb_Code && $gb_Table && as_Text[i, 1] == '|'
			ak_Out << '&'
		else
			ak_Out << as_Text[i, 1]
		end
	end
	lk_BlockScopes.reverse_each do |ls_Scope|
		ak_Out << $gk_BlockScopes[ls_Scope][:end]
	end
	ak_Out << '\\\\' if $gb_Table
	ak_Out << "\n"
	
	unless lk_Scopes.empty?
		puts as_Text
		puts lk_Scopes.to_yaml
		puts
	end
end

def handleSection(ak_Section, ak_Out, ai_Level = 0)
	lb_InEnumeration = false
	lb_InPre = false
	ak_Section.each do |lk_Section|
		if lk_Section.class == Hash
			#p lk_Section
			ak_Out << "\n\n\\section{#{lk_Section.keys.first}}\n\n" 
			handleSection(lk_Section.values.first, ak_Out, ai_Level + 1)
		elsif lk_Section.class == String
			ls_Text = lk_Section
			handleString(ls_Text, ak_Out)
		end
	end
end

lk_Out = ''

li_ChapterCounter = 0
lk_Guide['chapters'].each do |lk_Chapter|
	lk_Out << "\\cleartoevenpage\n"
	ls_Chapter = lk_Chapter.keys.first
	lk_Out << "\n\n\\chapter{#{ls_Chapter}}\n\n"
	lk_Out << "\\clearpage\n" unless ls_Chapter == 'When You Wish Upon a Beard'
	handleSection(lk_Chapter.values.first, lk_Out)
	li_ChapterCounter += 1
	#break if (li_ChapterCounter >= 4)
end

lk_Out << "\\end{document}\n"

# do some manual cleanup...
lk_Out.sub!('(with Cartoon Foxes)', '\mbox{(with Cartoon Foxes)}')
lk_Out.gsub!("``'", "```")
lk_Out.gsub!("`em.", "'em.")
lk_Out.gsub!("`em ", "'em ")
lk_Out.gsub!("`Cause ", "'Cause ")
lk_Out.sub!("\\section{The Continued Story of My Daughter's Organ Instructor}",
	"\\section{The Continued Story of My Daughter's Organ \\mbox{Instructor}}")
lk_Out.sub!("\\begin{tabular}{p{0.075\\textwidth}p{0.4\\textwidth}p{0.44\\textwidth}}\n\\multicolumn{3}{l}{\\bf  Quantifiers }", "\\begin{tabular}{p{0.075\\textwidth}p{0.3\\textwidth}p{0.54\\textwidth}}\n\\multicolumn{3}{l}{\\bf  Quantifiers }")


lk_OutFile = File.open('Proteomatic.tex', 'a')
lk_OutFile.print(lk_Out)
lk_OutFile.close

exit

system('pdflatex Proteomatic')
system('pdflatex Proteomatic')
