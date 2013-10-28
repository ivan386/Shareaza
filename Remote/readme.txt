Author: Jonne (2006)

Useful information if you're working on the remote
__________________________________________________
I tried to get as much as possible into external files, so loading
the remote is quicker as soon as your browser has cached the js and
css files. This also makes editing the remote itself easier, as you 
now only have to edit the css file to change the page style. You 
should be able to do all your styling needs through css, without 
having to touch the code. same with javascript: the .js is completely
independant, and doesn't rely on functions coded into the html.

If you want to add more external resource files, you *must* put them
in the '/images' folder. For some reason Shareaza is programmed to 
only get resources from there, so don't create a 'js','script','css'
or 'foo' folder because you prefer to have them seperated.

If you feel you hacked this remote into something useful, please post
your changes somewhere on the Shareaza forums. They might make it into
the next release (if they're really useful and crossbrowser).