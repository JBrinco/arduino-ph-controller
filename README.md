# arduino-ph-controller

 Simple arduino program for calibrating pH meters and controlling solution pH values via relays and valves.

## What is it?

This repository contains two scripts written for the arduino. They are meant to be used as specified in the article *TO BE ADDED*. You should setup the wiring and peripherals as explained in the article.

There are two operation modes: a single pH meter with two valves (one for adding acid and another for adding base) onto the same solution; And two pH meters each controling one valve. In the second case you have two different solutions, and can only add either acid or base to each (your choice). If you need to add acid and base to both solutions (4 total valves), a change in the code, wiring and box is necessary. An easyer way to do this would be to simply make two separate systems, each with one pH meter and two valves.

## What do the different files mean?

Each script has two similar files: a heavily commented one (single\_meter\_double\_relay\_commented.ino and double\_meter\_double\_relay\_commented.ino), and one with comments only on the header, to help you. Use which ever one you want, the arduino compiler doesn't care. I include them both because the heavily commented one is better for people withought experience (even with no programming experience, if you read the comments you should be able to get the gist of everything, I hope...) and the streamlined one is much simpler to read for people experienced in arduino. I put the commented ones in the folder "Documentation", to avoid confusion.


## How do I set things up?

Assuming you have read the article, all you need to do is change the relevant variables at the top of the file and upload it to the arduino. If you don't know how to use the arduino IDE, refer to https://www.arduino.cc/en/Guide/ArduinoUno . After opening the script you downloaded from this repository and changing the varibales according your needs, follow instructions starting in "Select your board type and port". After the code has been uploaded to the arduino you don't need the computer anymore, except for making changes to the varibles.

## It doesn't work!

Yep. It may happen. Well, firsly make sure all you connections are solid and in the right place. Then review the changes you made to the variables. If you can't figure it out, send me an e-mail! I'll help in what I can.

## Licensing and support

This small work is free software, licensed under the GNU General Public License Version 3. All details are in the LICENSE file, i'd encourage you to read it, but it is over 600 lines long... Just search GNU GPL if you want to know what you can and can't do with this code.

This work was released as part of a scientific project. If you want to support us, consider adding the controler to your own experiments and referencing our article! Also, feel free to send me an e-mail talking about your experience with the project, i'd love to hear from you!
