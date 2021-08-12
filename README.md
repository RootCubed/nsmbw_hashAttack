# NSMBW hash cracking: Finding out the lengths of function names and arguments

## Preamble
This writeup demonstrates an algorithm that allows for deriving the length of a function name and its arguments if you:
- Have found two functions which are identically named and take the same arguments
- Know the class names of the two functions
- Know the hashed value of the demangled and mangled symbol

## About the hashing process
The Chinese Shield TV version of New Super Mario Bros. Wii includes a list of the symbols of the main executable. However, the symbol names are hashed, but luckily Nintendo didn't use a particularly strong hashing algorithm, but rather a simple XOR-and-multiply algorithm (an implementation of which can be found [here](https://github.com/simontime/iQiPack/blob/master/crypto.cpp#L5)). For example, it is possible to undo a hash character by character. This document uncovers another "flaw" in this hashing algorithm.

## About hash collisions
A hash collision is when two different inputs produce the same hash. Every hashing algorithm will produce hash collisions, because it maps an arbitrarily long input to a finitely long output. However, with popular hashing algorithms like MD5 or SHA-1, collisions are essentially impossible to find. With the NSMBW symbol hashing algorithm, on the other hand, it is actually very easy to find hash collisions. Just hash a simple 4 letter string like "test" and exhaustively go though every string of length 4 and you will that find the string "uERt" has the same hash.

## The weak point
Because a hash can be undone character by character, you can do this, for example:

Take the two strings `NumberOne_secret` and `NumberTwo_secret`.

| Step | hash(str1) | hash(str2) |
|-|-|-|
| 1. Hash str1 and str2. | 0x0673af0b| 0xa0947263 |
| 2. Undo the string `_secret` from both hashes. | 0xbdf9c7e2 | 0xbdf9e44a |
| 3. Undo `One` from str1 and `Two` from str2. | 0xaa009926 | 0xaa009926 |

Even though the `Number` part hasn't been undone yet, the hashes already match up. This is to be expected from a hashing algorithm where you can undo it piece by piece (or should I say, byte by byte :p). What is more noteworthy though is the following:
| Step | hash(str1) | hash(str2) |
|-|-|-|
| 1. Hash str1 and str2. | 0x0673af0b| 0xa0947263 |
| 2. Undo the string `Ipwbfwt` from both hashes (more on this string in a bit). | 0xc4a1d622 | 0xc4a1f28a |
| 3. Undo `One` from str1 and `Two` from str2. | 0xce7d4ee6 | 0xce7d4ee6 |

Even though we didn't undo the correct string at the start, we still got the same hash! Note however that this is not a regular hash collision; hashing `NumberOneIpwbfwt` and `NumberTwoIpwbfwt` do not result in the beginning hash (For str1: 0xbe00394b != 0x0673af0b).

Here's how it differs: If you were to try to find a regular hash collision for, say `MySecretSymbolName` (0xb2656506), you would have to try random strings until one of them also has the exact hash of 0xb2656506. With this vulnerability, if you have `NumberOne{Unknown}` and `NumberTwo{Unknown}`, you have to find a string that, when used to undo both original strings until the end of `Number`, produces the same hash. That resulting hash can be any number though, not just some specific one. This makes such a hash collision more common and thus easier to find.

Now, here comes the trick: These hash collisions are not only common enough to be found when using randomly generated strings (Finding a matching string for the example, `Ipwbfwt`, happened almost immediately after beginning the search), but they also appear a disproportionately large amount of times when the colliding string has the same length as the correct one (only if your assumption of the static part of the name is correct though! Otherwise you might get never get the correct length).

## Application on the NSMBW hashes
Thanks to [Ninji](https://twitter.com/_Ninji), a large amount of the hashes have already been cracked. A group of uncracked hashes interested me specifically though, the last missing functions in `fBase_c`. In the `fBase_c` class, there are the `afterXXX` functions which get called after `preXXX` and `doXXX`. I used this method to:
1. Figure out that the enum name that gets passed to it has length 12 and belongs to the class `fBase_c`, and
2. Verify that the `afterXXX` functions are most likely actually called that (for afterDelete, the length of the name calculated via the hash is also 11 characters long)

### Further details on this
I used the fact that the afterXXX functions are thunked in `dBase_c` to get identical functions with different base classes. I started with the `afterCreate` symbol hash and used the hash for the mangled symbol to find out the length of the enum name. At first I assumed that the type name was simply `{len(name)}name` and quickly found collisions for 18 character strings. But I knew this wasn't correct, because I could not get any 18 character collisions with the `afterDraw` symbol hash.

I realized that the enum might be a class member and just assumed that it must then belong to `fBase_c`. I let the program find collisions for `7fBase_cFQ27fBase_c{len(enumname)}enumname` and was able to find collisions with length 12. I moved over to the demangled symbol names and wasn't able to get any matches for `fBase_c::{funcName}( fBase_c::{enumname} )`. However, I was able to find collisions with length 12 for `fBase_c::{funcName}( {enumname} )`.

~~This makes me rather confident in saying that the enum type passed to the `afterXXX` functions belongs to the class `fBase_c` and is 12 characters long.~~ `fBase_c::{funcName}( {enumname} )` would not be how the demangled symbol should look like, the `fBase_c::` part does not get stripped. I am continuing to look for properties of the enum type name.

## Code
This repository includes the code that I used to make these findings. `mainTool.c` is what can be used to figure out the length of function names and arguments. (I should probably make it a command-line tool instead of hardcoding all values though)