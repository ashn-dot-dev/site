It Would Be Nice if the "Disable Torment Nexus" Button Actually Disabled the Torment Nexus
==========================================================================================

Let's play a game of "Spot the Pattern".

**Grubhub** is a website that one can use to purchase food. It is a hub of
grub.

**StubHub** is a website that one can use to purchase tickets to entertainment
events. It is a hub of tickets that will (abstractly) become ticket stubs.

**Pornhub** is a website that one can use to watch porn. It is a hub of porn.

So if Git is distributed version control software, then **GitHub** would be...

{{Image src="/blog/2025-09-07-it-would-be-nice-if-the-disable-torment-nexus-button-actually-disabled-the-torment-nexus/2025-09-07-github-bluesky.png"}}
Screenshot of the GitHub Bluesky profile with the bio reading "The AI-powered
developer platform to build, scale, and deliver secure software."
{{/Image}}

...the hub of an AI-powered developer platform?

One would probably expect that a website called GitHub would advertise itself
as a hub for Git, right? I guess not.

----

Anyway, now that we are past that totally unrelated narrative hook, let's talk
about a frustrating encounter I had yesterday evening. So I am on GitHub
looking at the issue tracker on one of my public repositories, and I notice
this new "Development" section on the right hand side of the page with a button
that says "Code with agent mode". So I look into what this "Code with agent
mode" button means, and apparently the button is for the [GitHub Copilot coding
agent](https://docs.github.com/en/copilot/concepts/coding-agent/coding-agent),
some AI coding tool that is now in preview.

{{Image src="/blog/2025-09-07-it-would-be-nice-if-the-disable-torment-nexus-button-actually-disabled-the-torment-nexus/2025-09-07-github-issue.png"}}
Screenshot of the GitHub issue with a red box drawn around the "Development"
section of the page. Within that development section, there is a button that
says "Code with agent mode".
{{/Image}}

Now I personally do **not** want AI anywhere near my open source work. I think
that AI in its current form is a net-negative for humanity, and I do not want
any AI features enabled on a platform that I use strictly for version control
and issue tracking. So I am searching for a way to disable this "Code with
agent mode" button and I notice there is a "Copilot settings" button under the
GitHub profile dropdown.

{{Image src="/blog/2025-09-07-it-would-be-nice-if-the-disable-torment-nexus-button-actually-disabled-the-torment-nexus/2025-09-07-github-copilot-settings.png" style="height: 50vh"}}
Dropdown menu under my GitHub user profile with a "Copilot settings"
button/link.
{{/Image}}

I click that button and am taken to a GitHub Copilot settings menu with a "Show
Copilot" setting that I can set to "Disabled".

{{Image src="/blog/2025-09-07-it-would-be-nice-if-the-disable-torment-nexus-button-actually-disabled-the-torment-nexus/2025-09-07-github-copilot-visibility-disabled.png"}}
Settings menu with a "Show Copilot" setting featuring the setting description
"Enable Copilot for all GitHub features, including navigation bar, search, and
dashboard. When disabled, Copilot will be hidden and unavailable. This setting
does not apply to Copilot search on GitHub Docs." The options for the setting
are "Enabled" and "Disabled", of which "Disabled" is currently selected.
{{/Image}}

Great, so this will disable that "Code with agent mode" button, right? That
button in the repository issue tracker is for the GitHub Copilot coding agent,
and I just updated a setting to disable showing Copilot features. Well no, the
"Code with agent mode" button was still on the issue tracker! What the heck?!

Actually, this is half bad GitHub UX and half my fault. On the left hand side
of the settings page, the Copilot settings section actually has two sub-menus,
"Features" and "Coding agent". By default, navigating to "Copilot settings"
takes you to "Features". On my laptop, those two sub-menus are off-screen
without scrolling, and it was not obvious that there were two separate settings
pages. Anyway, navigating to that other "Coding agent" settings page, I see
another setting "Repository access" that I can set to "No repositories".

{{Image src="/blog/2025-09-07-it-would-be-nice-if-the-disable-torment-nexus-button-actually-disabled-the-torment-nexus/2025-09-07-github-copilot-repository-access-disabled.png"}}
Settings menu with a "Repository access" setting featuring the setting
description "Choose which repositories Copilot coding agent should be enabled
in. Copilot coding agent will only be available where it is enabled for the
repository and in the Copilot license policies." The options for the setting
are "No repositories" and "All repositories", and "Only selected repositories".
of which "No repositories" is currently selected.
{{/Image}}

Okay so surely this must disable that "Code with agent mode" button right? That
button in the repository issue tracker is for the GitHub Copilot coding agent,
and I just updated a setting to restrict Copilot coding agent repository access
to no repositories, which should be equivalent to disabling the Copilot coding
agent in all repositories. Well no, the "Code with agent mode" button was still
on the issue tracker! Again, what the heck?!

{{Image src="/blog/2025-09-07-it-would-be-nice-if-the-disable-torment-nexus-button-actually-disabled-the-torment-nexus/2025-09-07-trying-to-disable-the-torment-nexus.gif"}}
Starting in the "Coding agent" setting, "Repository access" setting is shown to
be set to "No repositories" before navigating to an "Issues" page where the
"Code with agent mode" button is still shown.
{{/Image}}

Maybe the "Repository access" setting is *somehow* still not the correct
setting to disable this "Code with agent mode" button. Maybe the engineering
team that implemented this feature shipped with a bug that still shows this
button even when the user explicitly asks to not be shown Copilot and
explicitly disables Copilot coding agent access in all repositories. Maybe
nobody at GitHub decided to test this user interaction, because everyone in the
organization believes GitHub Copilot is the future and no one would seriously
ever turn that feature off, right?

Whatever the underlying reasons are, from my perspective as a GitHub user, this
sucks. Every single day that I find myself in front of a screen, it seems
nearly impossible to escape from the flood of dystopian information related to
this AI hype cycle. OpenAI raises a bajillion dollars and buddies up with an
Authoritarian US President, all so that ChatGPT can [help a teen commit
suicide](https://www.nytimes.com/2025/08/26/technology/chatgpt-openai-suicide.html).
Anthropic agrees to [pay 1.5 billion dollars as part of a settlement in a
copyright infringement
lawsuit](https://www.nbcnews.com/tech/tech-news/anthropic-settlement-lawsuit-authors-rcna229422)
they found themselves in while building their state-of-the-art plagiarism
machine. Salesforce shows that it is "on the front lines of the AI revolution",
by [laying off 4,000
employees](https://www.cnbc.com/2025/09/02/salesforce-ceo-confirms-4000-layoffs-because-i-need-less-heads-with-ai.html).
I mean fuck even `notepad.exe` has a shitty "AI Features" setting section where
Copilot is enabled by default!

So when I go to GitHub, a website that I would like to use as a hub for Git
projects, and I am greeted with an AI slop button that does not respect my user
preferences, then I am going to be upset. It would be nice if the "Disable
Torment Nexus" button actually disabled the Torment Nexus.
