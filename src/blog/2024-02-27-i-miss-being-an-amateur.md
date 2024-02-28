I Miss Being an Amateur
=======================

> Do what you love, and you'll never work a day in your life.
>
> — person who has never worked as a software developer

I think that programming is fun. Or at least I think that I think programming
is fun. Or maybe I think that I *should* think that programming is fun, because
thinking programming is fun is what someone *would* think if they spent their
free time programming while also being employed as a full-time as a software
engineer. Okay this is getting out of hand, I think I need to take a second to
sort this all out...

I started programming recreationally years before I ever considered software
development as a possible occupation. A decent amount of my young adult life
was spent creating basic games, building simulations, and writing quaint
automation scripts for no other reason than "because I wanted to". It didn't
matter if the graphics were terrible. It didn't matter that the programs were a
mess of spaghetti code. It didn't matter if an application would crash when you
gave it unexpected input. Programming was *fun* and the labor of programming
was worth doing for its own sake.

At some point during my stint in higher education, I realized that I was
spending almost all of my free time programming instead of going to class or
doing homework. I was nearly halfway through an engineering degree when I made
the wise decision to change my major to computer science and get some formal
training in the thing that had at this point become my passion. I took
programming courses. I started reading programming books and the blogs of
popular software developers. I participated in my school's internship
program. And I started putting more effort into my programming side projects.
Through internships, and eventually full-time employment, I transitioned from
an amateur programmer to a bonafide software engineer. It would still take a
few years for me to grow and operate above a junior level, and it would take
even longer to get to the point where I considered myself to be a "good"
programmer, but somewhere along the way I became a professional, and
professionals have standards.

Professionals care about software architecture.
Professionals care about API design.
Professionals care about testing.
Professionals care about portability.
Professionals care about dependency management.
Professionals care about maintenance costs and technical debt.
Professionals care about deliverables and project planning.
Professionals care about a lot of stuff!
And if you are professional like me then it is incredibly difficult to get back
into the mindset of an amateur when working on hobby projects.

[Sunder](https://github.com/ashn-dot-dev/sunder) started as a purely academic
exercise that I intended to wrap up with my [Celebrating One Year of
Sunder](https://www.ashn.dev/blog/2022-02-19-celebrating-one-year-of-sunder.html)
blog post. But I ended up putting another two years and hundreds of hours into
the project because I *knew* it could be better. Even now, there is a list of
minor language-level faults[^1] in Sunder that bother me to no end even though
they are non-issues in practice. My current side-project, a [networked Settlers
of Catan clone](https://github.com/ashn-dot-dev/natac) has become *exhausting*
to work on. I overthink the trade-offs associated with every choice I make
instead of just building the damn game and having fun. Recently I stressed over
the details of byte-string escaping in a totally
non-production-grade-just-for-personal-use data interchange library. I know the
importance of internationalization, and some part of my brain needed to make
sure that `"Björn Ironside"` can appear verbatim as text, even though I work
almost exclusively with ASCII data in all of my projects.

When I was an amateur I didn't focus on  the little details so much. I was just
happy to build software that did something cool and worked for the set of use
cases I cared about during development. Programming was enjoyable because there
were no strings attached - all play and no work. Now that relationship is more
complicated, because even when there are no expectations, no goals, no
deliverables, no stakeholders, and no strings attached, it is difficult for me
to shut off the part of my brain that treats programming as job.

I miss being an amateur. I miss the ignorant bliss of not knowing the "right"
way to do things. I miss being able to fail without denting my ego. I miss
experimenting. I miss having separation between work and recreation. I miss not
feeling burnt out.

And just to be clear, programming is something that I still enjoy. Or at least
I *think* it is something that I still enjoy. It would be difficult to put so
many hours into software development as a hobby if I wasn't getting *something*
back out of it. But I am have a hard time telling how much of that enjoyment is
coming from a genuine love of creating something new versus how much of that
enjoyment is coming from me executing on a skill set that has become
comfortable to me. It's a really messy position to be in, and I'm not too sure
how to feel about it.

Perhaps spending some time away from computers during my recreational hours
will give me the space I need to sort out my relationship to programming. I
have other hobbies, some of which do not involve me sitting in front of a
screen. Cooking is something that I really enjoy even though I am pretty
terrible at it. I used to spend a couple nights a week long-distance running,
and that was really great for clearing my head. Before I went all in on
software development I used to spend a lot of time solving math and physics
problems[^2]. Maybe there is an opportunity to re-learn some engineering
fundamentals without the pressure of an academic institution or my career
development looming in the background. Maybe I should try to pick up a
completely new activity: knitting, or playing guitar, or studying economics, or
something else entirely.

I am not sure what the best course of action is here, but I do know that there
are other activities that can be fun besides programming. And maybe, just
maybe, one of those activities will let me re-capture that wonderful feeling of
being an amateur again.

## Footnotes
[^1]:
Specifically, the lack of order-independence for `extend` declarations, the
lack of a top-level `when` construct for conditional compilation, the inability
to use certain templated types such as `std::hash_map` and `std::hash_set` in
recursive type definitions, and the lack of strictly standards-compliant C code
generation, to name a few.

[^2]:
I spent an absolutely bonkers amount of time playing the early access version
of [Kerbal Space
Program](https://store.steampowered.com/app/220200/Kerbal_Space_Program/) as a
kid. This is the game that got me interested in engineering and rocket science,
and I actually remember that one of the first Python programs I ever wrote was
a thrust-to-weight ratio calculator to check whether my rockets would be able
to take off from other planets after landing.
