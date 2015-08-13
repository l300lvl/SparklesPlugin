# Sparkles
========

an XChat/HexChat plugin that makes it easy to be annoying on IRC

## Commands and features
=======

Sparkles makes a "( Network is [Network name], [Server URL] )" message when you join a channel, so that if the filenames of the logs get damaged or you just don't have the network in your list (so it gets recorded with the generic "NETWORK" name) you can still find out what IRC network the logs were from.

Sparkles can disable highlights, show a character counter, force UTF-8, clear out the colors of tabs after a nick change, automatically ghost, make Pesterchum easier to chat on.

=======
###/yiff
`/yiff Victim options options options`

Yiffs the specified person in a random way.

Option        | Effect
------------- | ------
 -fuk         | Yiffs retardedly but differently from davyiff
 -ms Species  | Uses some text in place of where a randomized species for you would've gone
 -ys Species  | Uses some text in place of where a randomized species for the victim would've gone
 -hstroll XX  | Filter the yiff action to be as if it were from a specific Homestuck troll
 -allcaps     | Filter the yiff action to be in all caps
 -randcaps    | Filter the yiff action to be in random caps
 -snuggle     | Use an alternate set of actions that doesn't contain any NSFW content
 -prefix      | Places some text at the start
 -emote       | Uses some text in place of the random emoticon on the end
 -conjoin     | Uses some text in place of ", and then" when chaining actions together
 -bold        | Sticks the control code for bold at the start
 -underline   | Sticks the control code for underline at the start
 -italic      | Sticks the control code for italics at the start
 -inverse     | Sticks the control code for inverse at the start
 -yc X        | Chain X number of actions together. Limited to 3.
 -shuffle X   | Shuffle X times per word
 -say         | Output will be in /say instead of /me so you can use it in /spark cmdstack
 -rainbow     | Add rainbows
 -list Name   | Specify a custom list

Custom lists are %appdata%/Sparkles/yiffName.txt and have one yiff per row.

Code | Effect
---- | ------
%y   | Target's nick
%m   | Your nick
%c   | Channel name
%s   | Your species
%S   | Target's species

###davyiff
`/spark davyiff Victim`

Yiffs the person with terrible grammar. Uses actual actions from dav_the_fox.

=======

###slashf
`/spark slashf Format`

Does a command with an alternative way of specifying control codes.

Code  | Effect
----- | ------
\i    | insert italics
\R    | insert inverse
\b    | insert bold
\u    | insert underline
\p    | insert clear formatting
\1    | insert 0x01
\\    | insert \
\a    | insert character 7 (bell)
\n    | insert newline
\r    | insert return
\t    | insert tab
\x__  | insert char by hex code
\h    | black spoiler
\w    | white spoiler

`/spark 2slashf Text`

`/spark 2slashfi Text`

Converts control codes to a format /spark slashf would take.
The latter puts it in the input box.

`/spark slashfs`

Like /spark slashf except it automatically adds "say"

`/spark slashfm`

Like /spark slashf except it automatically adds "me"

`/spark slashfi`

Like /spark slashf except it automatically adds "settext" so the result goes in the inputbox

###atloop
`/spark atloop Format Command Text`

`/spark atloops Format Text`

`/spark atloopm Format Text`

`/spark atloopi Format Text`

Sort of like /spark slashf, but loops through the format and takes codes differently (a vertical line sets the loop point):

Code | Effect
---- | ------
c    | Color code
cg   | Color code, rainbow cycle
cG   | Color code, rainbow cycle, doesn't advance position in cycle
u    | Underline toggle
i    | Italics toggle
b    | Bold toggle
R    | Inverse toggle
p    | Disable formatting
'X   | Insert character directly
_    | Insert space
*    | Copy any character from input to output
CN   | Switch to normal capitalization
CU   | Switch to all uppercase
CL   | Switch to all lowercase
CR   | Switch to random capitalization
?    | Copies all the way to the next isgraph() character and copies that character too
`?   | Copies all the way to the next isgraph() character
`c   | Inserts color code with no special behavior

=======
### repeatstring Times Text
`/spark repeatstring String`

Repeats Text as many times as Times specifies and says it

### space2newline
`/spark space2newline`

makes

you

talk

like

this

### rot13
`/spark rot13 Text`

Performs rot13 on text

### backwards
`/spark backwards Text`

Reverses text

### backwords
`/spark backwords Text`

Reverses the order of words in text

### bouncycaps
`/spark bouncycaps Text`

Capitalizes the first letter of every word.

### allcaps
`/spark allcaps Text`

Make-pretend you're Billy Mays

### nocaps
`/spark nocaps Text`

Lowercase everything

### altcaps
`/spark altcaps Text`

Capitalize every other character

### randcaps
`/spark randcaps Text`

Randomly capitalize

### acidtext
`/spark acidtext Text`

Does lots of crazy colors and stuff to the text

### accents
`/spark accents Text`

Changes characters to weird accented versions of themselves

### rainbowcaps
`/spark rainbowcaps`

A combination of "/spark rainbow" and "/spark randcaps"

### rainbow
`/spark rainbow Text`

Colors text to be rainbow colored. Use "/spark rainbow4" for changing colors only every 4th character.

### hstroll
`/spark hstroll Troll Text`

Filters text to be like that of a homestuck troll.
Use their two-letter trolltag abbreviation in specifying which troll you want.

### sellysay, scarletsay
`/spark sellysay Text`

`/spark scarletsay Text`

Alters text to be more like that of sellythefox or Scarlet from Sequential Art, respectively

### replwords
`/spark replwords ">WordA||<WordB" Text`

Replace a list of words

### replchars
`/spark replchars "aZeZiZoZuZ" Text`

`/spark replcharsi "aZeZiZoZuZ" Text`

`/spark replcharsim "aZeZiZoZuZ" Text`

For every character in the input, check against the first character in each pair, and if matched, replace it with the second.
The first is case sensitive, the second isn't, and the third keeps the case the same.

### shufflechar
`/spark shufflechar Times Text`

Swaps two letters in each word a specified number of Times. Does not do anything to the first and last letter of each word.

=======
### spawn
`/spark spawn Time Units Command`

Registers Command to happen after a specified amount of time. "Time" may
have a decimal point if you want. Use /spark spawnquiet for a version that
does not give a message after starting the timer.
"Units" may be d,h,m,or s for day, hour, minute, or second, respectively
Use "/spark spawn clear" to clear your spawn list.

### cram
`/spark cram`

Takes all the text in the input box and posts it all in a single line even if you copied and pasted several lines of conversation into it. Should probably assign it to a Userlist Button so you can do the command without entering it in the input box, or just use the Sparkles menu at the top. There's also "/spark cram noslash" which does not insert slashes between lines and "/spark crami" which puts the result in the input box.

### useinputbox
`/spark useinputbox Command`

Does Command with the current text in the input box after it. For example, doing "/spark useinputbox say OH HELLO " with "Billy Mays" in the input box would result in you saying "OH HELLO Billy Mays" in the channel.

=======
### ghost
`/spark ghost Nick`

Ghosts a nick using the NickServ password you have set for the network.

### ghost2
`/spark ghost2 Nick`

Ghosts a nick using the NickServ password you have set for the network, and sets your name to the nick you ghosted once it's gone. From Sparkles 0.97+ onwards it will change your nick even if NickServ complains the nick you're trying to ghost isn't connected.

### release
`/spark release Nick`

Releases a nick using the NickServ password you have set for the network.

### ident
`/spark ident`

Identifies with NickServ using the NickServ password you have set for the network.

=======

### 35font
`/spark 35font @0100 Text`

Says some text in a really big font. Some characters aren't in the font, and you can specify different colors than 01 and 00 if you want, or a different character than @ if you want, too. "RH" as a color becomes a horizontal rainbow, and "RV" a vertical one.

### x5font
`/spark x5font @0100 Text`

Like /spark 35font but does NOT take RH or RV, and uses a variable width font (to be more precise, it's the small font Texas Instruments uses on the TI-84+). You can use "r" for the config argument instead, to use reverses in place of colors. You can also use "rr" for the foreground color to randomize for each pixel. Alternatively use `/spark bigrainbow Text`

=======

### normalsay, normalme
`/spark normalsay Text`

`/spark normalme Text`

Says the given text without the sayhook or mehook

### sayhook, mehook
`/spark sayhook Command`

`/spark mehook Command`

Makes it so any message you type in at the bottom will use Command instead of /say. Mehook is the same but it does a /me.
Use "/spark sayhook off" or "/spark mehook off" to turn it off

### onesayhook
`/spark onesayhook SayhookCmd Command`

Like the above, but only does the sayhook'd command for one command only. Useful for commands that execute a bunch of other commands and you want to redirect them. Also works on /me, and there is /spark me2say which implicitly changes a /me to /say.

=======

### sparkencrypt1
`/spark sparkencrypt1 Your Text Here`

Says the text in the channel with a really simple encryption.

prefix that identifies something as encrypted: `02 16 1f 16 16 0f 02 03 30 30 03 30 30`

### run
`/spark run File`

Opens File and runs every single line of it as an XChat command. You do not need slashes.
There's "/spark run File quiet" also which doesn't announce anything.

=======

### SendModeSelect
`/spark SendModeSelect +X`

`/spark SendModeSelect -X`

Sets a mode on every nick you have selected in the channel nick list on the right.
(hold Ctrl while clicking to pick more than one)

### SendModePrefix
`/spark SendModePrefix +X Power`

`/spark SendModePrefix -X Power`

Like /spark SendModeSelect, except it selects who to set the mode on based on how much power they have. Specify +, @, &, or ~ in the "Power" parameter. (You can also use a period to specify people without anything). You can also stick in a list of people NOT to affect at the end, separated with spaces

=======

### getinfo, getprefs
`/spark get_info Info`

`/spark get_prefs Info`

Debug stuff. Fetches information, see XChat plugin reference for details

### set
`/spark set Group Item Value`

Set a specific config option. Yes/no and on/off are accepted for boolean options.

Option                        | Effect
----------------------------- | ------
General TextEditor            | Path to a text editor to use
Automatic ForceUTF8           | Force UTF-8 in all channels
Automatic CharCounter         | Displays a character counter if on
Automatic DisableNetworkSayer | Disable showing the network name upon joining a channel
Automatic DisableHighlights   | Disables highlights if on
Pesterchum ChannelCommand     | Command to use when speaking on Pesterchum channels
Pesterchum Color              | Color to use when speaking on Pesterchum channels (R,G,B)

### editconfig
`/spark editconfig`

Open up the config in a text editor, do `/spark rehash` to reload it

### openlogs
`/spark openlogs Channel Network`

Network can be omitted for the current one and Channel can too

=======

### invitejoin
`/spark invitejoin on/off`

Joins any channel you're invited to, if it's on

### rejoin
`/spark rejoin on/off`

Automatically joins channels you get kicked from again, if it's on

### autoident
`/spark autoident on/off`

If it's NOT turned on, Sparkles will automatically log in with NickServ when it whines

### autonickdeblue
`/spark autonickdeblue on/off`

If it's NOT turned on, Sparkles will automatically reset channel colors when you change nicks

### autoghost
`/spark autoghost on/off`

If it's NOT turned on, Sparkles will automatically reclaim your nick after you get disconnected.

### invite
`/spark invite eat/leave`

If set to "eat", invites won't be shown at all

=======

### repeatcmd
`/spark repeatcmd Times Command`

`/spark repeatcmd Times Delay Command`

Does Command a given number of Times, possibly with a Delay between each time (in seconds)

### rchan
`/spark rchan #Channel Command`

Does Command on #Channel. Will try it on the current network by default, or look in other networks you're in if there isn't a channel by that name in the current one.

### rchan2
`/spark rchan2 #Channel Command`

Does Command in ALL channels named #Channel

### mrchan
`/spark mrchan "#chan1!#chan2!#chan3" Command`

Like /spark rchan but in multiple channels

### multicmd
`/spark multicmd "cmd1||cmd2||cmd3"`

Performs multiple commands in one without needing /spark run

### pipe
`/spark cmdstack "cmd1||cmd2||cmd3" input`

`/spark pipe "cmd1||cmd2||cmd3" input`

Creates a chain of filters. `/spark cmdstack "spark backwards||spark rainbow" YourTextHere` would do a backwards rainbow.

### contextstack
`/spark contextstack push`

`/spark contextstack pop`

`/spark contextstack clear`

`/spark contextstack`

Push to, pop from, clear or get the current index of a stack of contexts

### cmdallchan
`/spark cmdallchan NqcspP YourCommandHere`

Does a command in every channel.

Flag | Effect
---- | ------
N    | Do command only on current server
q    | Do command in queries
c    | Do command in channels
s    | Do command on server tabs
p    | Do command only in channels you aren't in
P    | Do command regardless of if you're in or not (default is to have to be in it)

### chanclean
`/spark chanclean Nqcs`

Closes channels you parted from or were kicked from, or queries. Takes flags the same way as `/spark cmdallchan` except p and P are pointless

### unsettab, unsettabs
`/spark unsettab`

`/spark unsettabs`

Resets any tabs changed with /settab to their normal names. The second command affects all tabs.

### mathaddi, mathmuli
`/spark mathaddi Num1 Num2 Num3...`

`/spark mathmuli Num1 Num2 Num3...`

Get the sum or product of a list of integers you give

### hideset, hidecmd
`/spark hideset Option Value`

`/spark hidecmd Command`

Does a command but hides all output from the command that would normally be in the tab. The way it does this is pretty hackish, though.

### grabtopic
`/spark grabtopic`

Copies the current topic to the input box with colors and effects included.

### aexec
`/spark aexec Program`

Runs a program like /exec but asynchronously (only supported on Windows currently)

=======

### chancolorset
`/spark chancolorset N`

Sets the color number (0-3) of all channels to N.

### chancolorset2
`/spark chancolorset2 N`

Sets the color number (0-3) of all channels (on the current network only) to N.

### thou
`/spark thou Nick`

Shakespeare insult

### saveprefs
`/spark saveprefs`

Saves preferences to a .ini file

### randnum
`/spark randnum Limit`

Generates a random number
