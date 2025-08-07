How to Disable Visual Studio Code Shell Integration (Rant Edition)
==================================================================

TLDR - add this line to your `settings.json`:

```json
"terminal.integrated.shellIntegration.enabled": false
```

--------------------------------------------------------------------------------

You know what I find really annoying? I find it really annoying when an IDE
adds things to my personal configuration and system setup without my explicit
consent. You know what I find even more annoying? I find it even more annoying
when the additions that an IDE added to my personal configuration and system
setup without my explicit consent don't even work correctly.

Today I installed Visual Studio Code on my Mac and was greeted with the output
`bash: -: command not found` within the integrated terminal every time a
command was executed:

```sh
$ echo 'hello'
bash: -: command not found
hello
```

When I checked what was being executed with `set -x`, I noticed that Visual
Studio Code had added a ton of bloat around the actual executing command:

```sh
$ echo 'hello'
++ __vsc_preexec_all -x
++ '[' 0 = 0 ']'
++ __vsc_in_command_execution=1
++ __vsc_preexec
++ __vsc_initialized=1
++ [[ ! echo 'hello' == __vsc_prompt* ]]
++ '[' 1 = 1 ']'
+++ builtin history 1
+++ sed 's/ *[0-9]* *//'
++ __vsc_current_command='echo '\''hello'\'''
++ __vsc_command_output_start
++ [[ -z 1 ]]
+++ __vsc_escape_value 'echo '\''hello'\'''
... a bunch of looping execution
+++ builtin printf '%s\n' /Users/ashn/sources/somerepo
++ builtin printf '\e]633;P;Cwd=%s\a' /Users/ashn/sources/somerepo
++ __vsc_current_command=
++ '[' 1 = 0 ']'
++ __vsc_first_prompt=1
++ __vsc_update_prompt
++ '[' 1 = 1 ']'
++ [[ \[\]\w\[\e[33m\]`git_ps1`\[\e[m\]\$ \[\] == '' ]]
++ [[ \[\]\w\[\e[33m\]`git_ps1`\[\e[m\]\$ \[\] != \\\[\]\6\3\3\;\A\\\\]\\\w\\\[\\\e\[\3\3\m\\\]\`\g\i\t\_\p\s\1\`\\\[\\\e\[\m\\\]\\\$\ \\\[\]\6\3\3\;\B\\\\] ]]
++ [[ \[\]> \[\] == '' ]]
++ [[ \[\]> \[\] != \\\[\]\6\3\3\;\F\\\\]\>\ \\\[\]\6\3\3\;\G\\\\] ]]
++ __vsc_in_command_execution=0
++ __vsc_update_env
... actual contents of my PS1 executed here
```

To prevent Visual Studio Code from adding all of this extra crap to every
command, disable Visual Studio Code shell integration with the setting
`"terminal.integrated.shellIntegration.enabled": false` in your `settings.json`
or lookup `terminal.integrated.shellIntegration.enabled` in the settings menu
and uncheck the `Terminal -> Integrated -> Shell Integration: Enabled` setting.

![terminal.integrated.shellIntegration.enabled setting](/blog/2025-08-03-vscode-disable-shell-integration/setting.png)

I understand that Visual Studio Code is a free (as in beer not as in freedom)
product that I willingly downloaded onto my machine. I understand that there
were probably a good number of engineering hours put into this shell
integration. I even understand that this level of shell integration is
something that a lot (most?) developers will want by default when using Visual
Studio Code as an IDE. But there is something about this that rubs me the wrong
way.

When I interact with a terminal, I want to use **my preferred shell** with **my
specific shell configuration** executing only the commands given on **my
behalf** with **my explicit consent**. I do not like when a company or a
product opts me into something I was unaware of. If I hadn't stumbled upon that
`bash: -: command not found` error message, then I probably would not have
known that this shell integration was even happening. Microsoft's choice of
default IDE settings is arguably not worth getting upset over, but this
happened to hit at just the right time for me to get maximally grumpy to the
point where I needed to rant!
