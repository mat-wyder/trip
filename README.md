<p>
<u><b>The Random Instruction Parser v0.5</b></u><br>
</p>

<p>TRIP is an interpreter aiming to provide a completely redefinable
language. Most modern environments offer some level of flexibility,
you can do lots of interesting stuff with lex/yacc, but as far as I know,
none of these let you directly mess with the bare DNA of the beast, from inside
the running code itself. Here it is all about <u>how</u> you
define the language: by the language itself, at run-time, step-by-step
in your code, and when you feel like it, you just define new operators,
re-define or rename older operators, swap priority levels... get lost,
get confused, feel stupid, get angry, wipe out the whole mess and live
a happy and fulfilling life knowing you are never required to touch
this thing again. What else could anyone ask for? In the end, it's just for fun!
</p>

<p>The following is my early effort to try to explain what is happening.
You might want to consider looking at default.ri or playground.ri instead,
that may or may not give you a better grasp than what you are about to read.
Or just scroll down to the "default symbols" section. I warned you, it will
get really weird.
</p>


<p>Are you set?</p>

<p>TRIP by itself hasn't got fancy constructs, only the bare minimum. You get
weakly typed integers, basic math, basic logic, true & false (try renaming/swapping them,
and/or the and/or operators, it's a weirdly satisfying joy to create bugs
on intent), you got a ternary style if-then-else junction, a loop, and pretty
much that's all for now. There are no variables, functions, etc. - everything
is represented by a generalized form of data, <u>symbols</u>.

By default, all symbols are somewhat isolated, every one of them in ASCII
with \_surrounding\_underscores\_, hence parsing an ordinary text file most
likely does not ring any bells, so if you are up to some quick and low
performance (!) stream editing, replacement task or more awk-ward stuff,
just write a short script satisfying your needs, and include your subject
afterwards. At least that's one possible use case once some of the quirks
are resolved.

<p>
Let's start with a few examples (assuming default.ri loaded):<br><br>
<b>"{+}~={*}; 5+5"</b> (set + to be *) will result 25<br>
<b>"{+}#s##g#{*}+1; 100+1*20"</b> (set the
priority of addition over the priority of multiplication) yields (100+1)*20=2020
instead of 100+1*20=120<br>
<br>
which is stupid from almost all points of view, but wonderful freedom from
another. At least that's what I thought back when I put it together. Note that
this affects the whole entireity of the global scope (no other scopes around
for now, this is among the issues to be solved, without sacrificing the
simplicity)<br>
</p>

<b>@:={ $a:=(\_L\_); $b:=1; $res:=1; $b<=$a @> ({ $res*=$b; $b++; }); $res }; 5@</b>
returns 5! = 1*2*3*4*5. Of course it can be defined as "!" but then you'll need to rename the logical "not" symbol.
Note the ({ block of code }) construct after the loop ( @> ) symbol, this is how you can build
hierarchy.<br>


<p>
A symbol's content by default is treated as code, but there is an escape mechanism
to get around this - the same you use to organize code in blocks. Every
symbol you assign can have the strings <b>"\_left\_"</b> and
<b>"\_right\_"</b> (default.ri renames these to
\_L\_ and \_R\_ respectively) in their content; these reference
to strings to the left and right sides of the symbol whenever found in
the code. Normally these are escaped (using \_iEL\_ and \_dEL\_, or curly braces in
the default.ri case), which can be countered by using \_iPL\_ and \_dPL\_, or
parentheses for default.ri<br>
<br>
<b>{1+2}</b> returns "1+2", from many aspects
you can think of it as the quotes in other languages. <br>
<b>({1+2})</b> evaluates the escaped
part, resulting in "3". This behavior can be used to create blocks of code.<br>
<br>

A symbol is a set of three values: <u>case</u> (for example, in the default
instruction set <font style="font-weight: bold;">"\_assign\_"</font>)<b>,</b>
<u>code</u> and <u>priority</u>.

Priorities (signed integers in current release) are&nbsp;the method for
TRIP to determine the order of execution; a symbol with a smaller
absolute value of priority executes prior to symbols that have greater
absolute value of priorities.

As each symbol parses the whole instruction scope it sees during execution,
you have to take care the mentioned \_left\_ / \_right\_ symbols are properly
separated by higher order (smaller priority value) symbols responsible
for proper program flow.


For example, the default <b>"\_noDo\_"</b>
symbol has the smallest preset priority (1024); it is used to split the code
in smaller chunks that execute one-by-one, from left to right (typically
this is the semicolon character in other languages). The <b>"\_assign\_",</b> <b>"\_mAdd\_",</b>
etc. symbols have greater priority to execute after <b>"\_noDo\_".</b>
To make things clear (?), here's an example: in <b>"a\_assign\_10\_noDo\_b\_assign\_20"</b>
the
first symbol found is <b>"\_noDo\_"</b> - it does what it is for,
evaluates
the left side <b>"a\_assign\_10",</b> then the right side <b>"b\_assign\_10"\_noDo\_</b>
In
the latter two expressions, <b>"\_assign\_"</b> is found and the letters
become symbols with numbers in their code part.</p>
<p>TRIP evaluates an expression until it's return value doesn't contain
any symbols; for example, the code <font><b>"a\_assign\_b\_noDo\_b\_assign\_c\_noDo\_c\_assign\_10\_noDo\_a"</b></font>
returns 10 (as the result of the last "command", <b>"a",</b> that
evaluated to <b>"b",</b> a symbol, which had the value <b>"c",</b>
another symbol, which had the value <b>"10",</b> which is not a symbol
(altough it could be one, if I wrote <font><b>"a\_assign\_b\_noDo\_b\_assign\_c\_noDo\_c\_assign\_10\_noDo\_10\_assign\_20\_noDo\_a"</b></font>,
the return value would be then <b>"20"</b>). Another very important
fact is that TRIP has no concept of "whitespace", so you must
take care of spaces, tabs, enters too (usually by assigning them
an empty value, and setting their priority to <span
 style="font-weight: bold;">"\_noDo\_"</b>'s priority plus
one; that's done in the default init script, but you can do
anything else with them, it's up to you, again); This fact implies
another rule, that you must be aware: as there's no whitespace, nothing
tells the interpreter where a symbol must begin or end, it simply
matches it everywhere it is found - so avoid short names of symbols,
and <u>never</u> use one-character symbol names unless you want
exactly this to happen: <font><b>"g\_assign\_b\_noDo\_oo\_assign\_a\_noDo\_good"</b></font>
will
evaluate to <font><b>"bad"</b></font>, which is not good indeed..<br>
</p>
<p>The <u>code</u> part of symbol is evaluated as soon as it is next in the priority
order, until non-symbol result got back (or the content is escaped, when one
level of braces removed and the content is returned), whenever they're
read in the code.
The code of a symbol may include any number of these two
context-reference symbols: <b>\_left\_,
\_right\_</b>. The <b>\_left\_</b> and <span
 style="font-weight: bold;">\_right\_</b> will be replaced with
everything on the left and right sides of the symbol, surrounded by
escape brackets (default \_iEL\_ and \_dEL\_).<br>
If neither side is referenced in the code, it initiates a replacement
(without escaping), and that's
evaluated again - for example, that makes <b>"%\_assign\_\_mAdd\_\_noDo\_5%6"</b>
evaluate
to 11. Here <b>"\_mAdd\_"</b> will not
execute on assignment, as it has nothing on both sides, it just returns itself, that
is assigned to <b>"%",</b> then it is
called with the numbers, which concatenated with <span
 style="font-weight: bold;">"\_mAdd\_",</b> will executed again.<br>
</p>
<p>There's a bug (design flaw) for which first I made a workaround, but
it made things&nbsp;very slow, and&nbsp;it's easy to avoid if you make
sure nothing executes prior to the "\_noDo\_"; if something does, it
taints&nbsp;the execution order (unless you want to do exactly this),
and weird things start to happen. For example if you try to do
whitespace-to-emptystring conversion with priority 1, the
interpreter will find all whitespace prior to anything, and without the
escape braces properly assigned/renamed, if there are symbol assignments
with context-reference (assigned to values that contain&nbsp;\_left\_ or
\_right\_), the evaluator will pick up those references just to replace
every occurrence with their respective left and right sides, basically
the whole script except that very space character currently "running",
and the process repeats over and over. Here's an example of this:</p>
<p>
<b>"\_iEL\_</b><br
 style="font-weight: bold;">
<b>\_dEL\_\_assign\_\_noDo\_\_iEL\_<br>
<b>\_dEL\_\_setPri\_1\_noDo\_<br>
iELwrap\_rename\_[[\_noDo\_</b><br style="font-weight: bold;">
<b>dELwrap\_rename\_]]\_noDo\_<br>
&lt;\_assign\_\_lLT\_\_noDo\_<br style="font-weight: bold;">
</b>&gt;\_assign\_[[\_right\_&lt;\_left\_]]\_noDo\_<br>
$a\_assign\_20\_noDo\_"<br>
</b></p>
<p>The code part of &nbsp;<b>"&gt;"</b>
will be <b>"$a\_assign\_20\_noDo\_&lt;\_left\_",</b>
which is not what we want. To avoid, we <span
 style="font-weight: bold;">\_setPri\_</b> the newline to <span
 style="font-weight: bold;">"\_getPri\_\_iEL\_\_noDo\_\_dEL\_\_mAdd\_1",</b>
(the priority of the noDo operator + 1)
this way it will not affect the order of
execution, and everything will work fine (Just joking. I have serious doubts
about anything more complex than a simple one instruction loop to work "fine"
in this environment. But hey, I said challenge!).</p>
<p>Another effect of this flaw can be seen in this&nbsp;example: <span
 style="font-weight: bold;">"%\_assign\_[[\_right\_\_left\_]]\_noDo\_\_right\_%A"
</b>expected
to return <b>"A\_right\_",</b> instead it evaluates to <b>"AA".</b>
This is because when first the <b>\_left\_</b>
symbol is replaced in the code,
it is replaced by <b>\_right\_,</b> so
the intermediate content of the
expression becomes <b>"\_right\_\_right\_",</b>
that correctly evaluates further
into <b>"AA"</b>.
<br><br>
(Sorry guys, most of this was written more than a decade ago, and my
brain already hurts, so... good luck, go figure a workaround. I'd start with
the actual source code, #1 issue to solve is scoping, which currently does
not exist at all. Copying the whole symbol structure on every call is dumb,
symbols could get another field for "execution level", I dunno, currently
it seems pointless to waste time on this. In the unlikely case this weird
thing draws any kind of interest, feel free to contact me.)<br>
&nbsp;</p>







<br>
<p>
<u><b>The basics:</b></u><br>
</p>




<p>You get the interpreter and a really spartan console built on top of it
(There's no line editor. You can type stuff that gets evaluated); you can
control it's behavior with the following command line switches:<br>
<br>

-n (no console): disable the console, only the file(s) specified will
be parsed, then TRIP exits.<br>
-w (warnings off): this is generally a bad idea when writing your
program<br>
-s (silent mode): no output<br>
-d (debug mode, new in v0.5): every single evaluation will be logged
on screen.
<br>
<br>




<p>
<u><b>Default symbols:</b></u></p>
<p>Default symbols in TRIP are all strings with the first and last
character "\_", they have a somewhat usual priority order, and are meant
to be renamed for a convenient environment - the default script sets up
this to some extent; You have a bad choice and a worse one: if you rename them,
you lose compatibility. If you write your own alternatives which reference them,
you lose performance (lol). The default symbols always evaluate their "operands" - that's
not always the case with user-defined
symbols, yet they behave the same way to the lack of operands - they won't execute,
just return their name.</p>
<p><b>"\_noDo\_"</b> - It is the <b>";"</b>
in most languages, <b>":"</b> or
newline in
basic, <b>";"</b> or newline in bash,
etc. It's purpose is to delimit
expressions which aren't related in special ways, and to provide linear
execution from the leftmost expression, in right direction. Could that be reversed?
Of course. Why would anyone do that? I don't know. But it is definitely possible.</p>
<p><b>"\_sThen\_" </b>-
ternary, use as: <b>&lt;condition&gt;\_sThen\_&lt;run this if condition is true&gt;\_sThen\_&lt;else this&gt;</b>
<p><b>"\_sWhile\_" </b>-
loop, use as: <b>&lt;condition&gt;\_sWhile\_&lt;execute this&gt;</b>
<p><b>"\_assign\_"</b> -
symbol assignment (creation): it takes the string from the evaluation
of the left side, and sets a symbol with that name, with the code part
taken from the evaluation of the right side\_noDo\_ <b>"a\_assign\_b" </b>sets&nbsp;symbol
"a"'s code to "b"\_noDo\_ but, if "a" was a symbol, and had a value, that
value will be the name of the new symbol. To assign new values to
existing symbols, you must escape the left side: <b>"a\_assign\_b\_noDo\_b\_assign\_c"</b>
will assign the value "c" to the symbol&nbsp;"a". Assignment returns
the value that was assigned, so <font><b>"a\_assign\_b\_assign\_10"</b></font>
will make both "a" and "b" have the value 10. If the symbol already
exists, it preserves its priority\_noDo\_ else, if the right side is a
symbol
(not just contains a symbol, but IS exactly a symbol), priority will be
inherited from it, else priority will be 0x7FFFFFFE, the highest value
possible (0x7FFFFFFF is for inactive symbols, i.e. they are not evaluated
or even found at all).</p>
<p><b>"\_rename\_"</b><b> </b>- rename the left side to the right side</p>
<p><b>"\_iEL\_" </b>and<b> "\_dEL\_" - </b>"increase/decrease
Escape Level" - the escape braces, nesting is possible, and each
evaluation removes one pair of braces from an expression. They prevent
evaluating what's closed between them. There are the&nbsp;<b>"\_dELwrap\_"
</b>and<b> "\_iELwrap\_"</b>
symbols which evaluate to \_iEL\_ or \_dEL\_, without these, renaming or
assigning \_iEL\_ or \_dEL\_&nbsp;would be impossible.</p>
<p><b>"\_setPri\_"</b> and <span
 style="font-weight: bold;">"\_getPri\_"</b>
- set or get priority of symbols: <b>"&lt;symbol&gt;\_setPri\_&lt;long&gt;"
</b>and <b>"\_getPri\_&lt;symbol&gt;"
</b>For the current version, default symbols have&nbsp;1024..1024*15
as
their priority, they have more than enough space between them to
achieve maximal&nbsp;flexibility (let you define symbols between them).
<b>"\_getPri\_"</b> should be used to check whether a symbol exists or
not; symbols can't have priority 0, so if <b>"\_getPri\_"</b> returns
with 0, the symbol does not exist.</p>
<p><b>"\_iPL\_", "\_dPL\_" </b>-
increase/decrease Priority Level, these are to&nbsp;override priority
order; Evaluate what's inside, then substitute into original context,
and eval again. Depending on the priority of the<br>
</p>
<p><b>"\_lOR\_", "\_lAND\_", "\_lNOT\_",
"\_lEQ\_",
"\_lLT\_" </b>- logic symbols, I think there's no need to explain
what are they used for (only thing to note, they work with the internal
"\_true\_" and "\_false\_" symbols)</p>
<p><b>"\_mAdd\_", "\_mSub\_", "\_mMul\_",
"\_mDiv\_"</b>
- integer arithmetic - first I tought it would be elegant to define
these by TRIP code, but it would be then slow like hell, and if you
want, you can still use customized symbols for your needs; for these, I
used the strtoul(); function and 4-byte machine native arithmetics,
they
are in correct priority order, ready-to-use. For those, who don't know
what strtoul() it is: it's a standard C library function that returns a
long from a char*'s
first numeric part - for example, 34a10 will be 34 for these symbols,
except if
"a" is a symbol with a numeric return value.</p>
<p><b>"\_sFind\_", "\_sChop\_"</b> -
string manipulation; these can be used to modify/morph existing code,
by&nbsp;the <b>"\_getCode\_"</b> builtin. Use: <span
 style="font-weight: bold;">"&lt;string&gt;\_sFind\_&lt;string\_to\_search\_for&gt;\_sFind\_&lt;N&gt;"</b>
returns the position of the first char of the "abs(N)"-th (from left in
N is positive, from right if negative) occurence of the searched string
- if there are less than N such strings, it returns the length of the
string. If the second "\_sFind\_" is omitted, 1 is the default value (the
first match from left).<br>
<b>"&lt;string&gt;\_sChop\_&lt;M&gt;\_sChop\_&lt;N&gt;"
</b>returns the left string's part, beginning from char# M, with a
length of N. You can write <b>"&lt;string&gt;\_sChop\_&lt;M&gt;"</b> to
use all characters after #M.<br>
</p>
<p><b>"\_chID\_"</b> - (char inc-dec) -
the number that represents the left side's last character (ASCII code)
will be incremented by the number on the right side (negative values
decrement).<br>
</p>
<p><b>"\_sLen\_" </b>-<span
 style="font-weight: bold;"> "\_sLen\_&lt;string&gt;"</b>
- return the length of a string, left side is dropped (may&nbsp;change
this behaviour&nbsp;in next version)</p>
<p><b>"\_getCode\_"</b> - returns the code of the symbol on the right, or
the string on the right itself, if no symbol with that name found.</p>
<p><b>"\_getFile\_"</b> - returns the contents of the file on the right
side. Used inside <b>"\_iPL\_"</b> and <span
 style="font-weight: bold;">"\_dPL\_",</b> it works like #include in
C, but you can do it anywhere in your program.
</p>
<p><br>
2004, 2020, GrooveNet<b><br>
Released under the terms of the GNU GPL v3</b></p>
