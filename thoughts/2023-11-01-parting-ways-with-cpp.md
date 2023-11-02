Parting Ways with C++
=====================

High Level Unorganized Thoughts:

+ I recently returned to C++ after two years of being away from the language.
+ After spending some time req-acquainting myself with the language, I have come to realize that I do not enjoy writing C++ anymore, and I do not have a desire to write C++ code in the future.
+ Having spent the last two years of my life mainly working in C, Python, and Rust, I see C++ as an unnecessarily complex expert-only language with a doomed future destined as a "legacy-only" systems language.
+ Although C has plenty of pitfalls, I enjoy the simplicity of writing C and would pretty much always choose it as an alternative to C++ in the relevant domains.
+ Python is comfy enough at this point that I don't see the high level language features added to C++ as being worth the costs in undefined behavior and cognitive load (ranges are too little too late, I don't care about concepts, I don't care about coroutines).
+ Rust is in a stable enough state that it can perform well in many of the same roles as C++, and although I am not a huge fan of Rust, the language and tooling (rustc + cargo) are much better than that of C++ (primarily CMake).
+ C++'s safety story is pretty much non-existent, and Bjarne's ideas to add profiles to the language are too little too late.
+ Bjarne seems scared of Rust replacing C++, and I think he should be; Rust has a lot to work out but it is here and stable today, and all of the proposed solutions to C++ would probably not be available and in wide use for another decade.
+ I have a life to live, and the most important things for me are my family, and my own physical and mental health - dedicating my life and carrier to be an expert in what I think is kind of a shit language is completely unappealing to me at this point in my life.
+ The few tasks that I tended to use C++ for can now be done with Sunder - and I view Sunder as a better successor to C than C++ for my own personal goals.
+ C++ has had enough recent scandals that I do not want to be a part of that community - it gives me a bad feeling in my gut and I have to trust my gut on feelings of ethics.

--------------------------------------------------------------------------------

I recently returned to writing some C++ after nearly two years away from the
language, and I have be so thoroughly disenchanted that I am removing C++
resume and will no longer be seeking employment opportunities in C++.

For some context, C++ was the first compiled language that I "learned" way back
when I was first starting programming. Before starting with C++ I had mostly
been programming in Python and MATLAB. At the time, C++ seemed so cool because
it was low level, let you talk to your machine directly (lol I know how far
from the truth that actually is now, but did not know back then), and was used
by professionals for high performance computing in things like game engines,
scientific computing, and trading systems. From my perspective, C++ was the
language that serious programmers used to write software, and if I wanted to be
a serious programmer then *I* should be using C++ to write software.

I spent a lot of time during my freshman year of college learning C++, writing
(bad) C++, watching talks on C++, and thinking about C++. I was still a novice
programmer at the time, so I did not have the wisdom to know out what features
were good, what features should probably be ignored, what concepts were
fundamental to software engineering and what was specific C++ fluff, but
overall I enjoyed that learning period of my programming journey. Learning C++
eventually introduced me to C, and my combined C and C++ knowledge is what lead
me to my current role as a systems programmer and compiler engineer. So in some
ways I think that C++ was instrumental to making me the software developer that
I am today.

After I discovered C I somewhat fell out of love with C++. Specifically, Mike
Acton's CppCon talk [Data-Oriented Design and C++](https://www.youtube.com/watch?v=rX0ItVEVjHc)
was and continues to be one of the most influential talks I have ever watched,
and is what started me down the patch of focusing on solving problems rather
than the towers of abstraction that the C++ language and standard library
nudges you into building. I kept C++ in my software development toolbox, and
continued to develop C++ professionally, but I never really found the same joy
writing C++ that I did writing C.

Then, nearly two years ago, I changed jobs to what is now my current position,
which mainly consisted of C development with some Python and some Rust as well.
Critically, the position involved zero C++, and in general C++ is not used as a
primary language at the company. At this time Sunder was also finally getting
to the point where it was usable as a general-purpose language, and so there
was also not really a reason for me to use C++ outside of work when I would
rather write software my own language.

Being out of practice for two years made me forget way more C++ than I
expected, and when I returned to the language for some recreational programming
(a course on modern OpenGL and some competative programming problems) I was
shocked at how difficult it was to pick the language back up, and I was shocked
with how much I disliked the language and tooling. I think the simplicity of C,
the amazing tooling of Rust, and the joy of Sunder, have spoiled me over these
past few years. C++ no longer fills any roles in that toolbox for me. Although
Rust isn't my favorite systems language, it is a hell of a lot better than C++
for for the problem domains that C++ typically excels in. Game development is
way more fun in Sunder than it ever was in C++ for me, and I can't imagine
myself ever doing gamedev with C++ again. And for Advent of Code type problems,
I would much rather use Sunder than C++, or (if Sunder would be a bad fit for
the problem) just use Python, a language that can express higher level code
with way fewer footguns.

I just don't really see a role for C++ in my life anymore. I don't have a
desire to re-acquaint myself with the language. I don't need it for work, and
can reasonably avoid it for future employment opportunities if desired. And
honestly I just think the language and community are kind of a dumpster fire
right now. The C++ standards committee doesn't seem like they have a plan to
compete with Rust, there has been drama after drama within the C++ community,
and as I look towards the future I can't see a path in which C++ doesn't become
a legacy language within my lifetime. C++ will probably outlive me, but I don't
think it is a language and ecosystem that I would like to be around for the
rest of my time as a developer, so I am respectfully parting ways and wishing
C++ the best of luck in whatever future awaits it.
