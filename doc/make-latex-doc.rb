require 'yaml'
require 'fileutils'

ls_Lines = File::read('proteomatic.txt')
lk_Lines = Array.new
ls_Lines.each do |ls_Line|
    lk_Lines << ls_Line
end

FileUtils::cp('doc-head.tex', 'Proteomatic.tex')

$gk_Rules = Hash.new
$gk_Rules['italic'] = {:char => '_', :start => '{\em ', :end => '}'}
$gk_Rules['bold'] = {:char => '*', :start => '{\bf ', :end => '}'}
$gk_Rules['quote'] = {:char => '"', :start => '``', :end => "''"}
$gk_Rules['code'] = {:char => '@', :start => '\inlineCode§', :end => '§'}

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
$gk_BlockScopes['h1'] = {:char => 'h1. ', :start => "\\cleartoevenpage\n\n\\chapter{", :end => "}\n\\newpage\n"}
$gk_BlockScopes['h2'] = {:char => 'h2. ', :start => "\n\n\\section{", :end => "}\n\n"}
$gk_BlockScopes['h3'] = {:char => 'h3. ', :start => "\n\n\\subsection{", :end => "}\n\n"}
$gk_BlockScopes['h4'] = {:char => 'h4. ', :start => "\n\n\\subsubsection{", :end => "}\n\n"}

$gb_List = false
$gb_Table = false
$gb_Code = false

def handleString(as_Text, ak_Out)
    as_Text.rstrip!
    
    # handle images
    if as_Text =~ /\A![<>_\*]?.+/
        ls_Name = as_Text
        ls_Direction = as_Text[1, 1]
        ls_Direction = '' if ls_Direction !~ /[<>_\*]/
        ls_Name.gsub!(/\A![<>_\*]?/, '')
        ls_Name.gsub!(/\(.*\)/, '')
        ls_Description = $~.to_s
        ls_Description.slice!(0, 1)
        ls_Description.slice!(ls_Description.length - 1, 1)
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
        ls_Caption = ''
        ls_Caption = "\n\\caption{#{ls_Description}}" unless !ls_Description || ls_Description.empty?
        if (ls_Direction.empty?)
            ak_Out << " \\begin{figure}[htbp]\n\\centering\n\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Destination}}#{ls_Caption}\n\\end{figure}"
        else
            if (ls_Direction == '_')
                ak_Out << "\\vfill\n\n"
                ak_Out << "\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Destination}}#{ls_Caption} \n\n"
                ak_Out << "\\clearpage\n\n"
            elsif ls_Direction == '*'
                ak_Out << "\\vspace*{0.6cm}\n\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Destination}} \n\\newpage\n"
            else
                ls_Caption = ''
                ls_Caption = "\\piccaption{#{ls_Description}}" unless !ls_Description || ls_Description.empty?
                ak_Out << " #{ls_Caption}\\parpic[#{ls_Direction == '<' ? 'l' : 'r'}]{\\includegraphics[width=#{lf_Width}\\textwidth]{#{ls_Destination}}} \n"
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
            ak_Out << "\n\\begin{lstlisting}"
            $gb_Code = true
            next
        end
        if (as_Text[0, 6] == '</pre>')
            i += 5
            ak_Out << "\\end{lstlisting}\n"
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

lk_Out = ''

lk_Lines.each do |ls_Line|
    handleString(ls_Line, lk_Out)
end

lk_Out << "\\end{document}\n"

lk_OutFile = File.open('Proteomatic.tex', 'a')
lk_OutFile.print(lk_Out)
lk_OutFile.close

system('pdflatex Proteomatic')
system('pdflatex Proteomatic')
