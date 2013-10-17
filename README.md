libtprpg
========

A tiny pseudorandom permutation generator library

DESCRIPTION
===========

libtprpg implements a pseudorandom permutation generator based on the
generalized feistel cipher algorithm described by John Black and Phillip
Rogaway in their paper "Ciphers with Arbitrary Finite Domains". See also

http://blog.notdot.net/2007/9/Damn-Cool-Algorithms-Part-2-Secure-permutations-with-block-ciphers

I make no claims as to the security of this implementation, it was designed to
be simple first and foremost. The round function is based on XTEA and the key
scheduling algorithm is based on RC4. Both XTEA and RC4 have know issues,
though libtprpg uses them in a way which should prevent any known attacks from
being applicable. One could easilly change the KSA and/or round function - a more
conservitive but still small/simple design would use siphash as the round function
and hkdf/siphash as the KSA.

USAGE
=====

Take a look at tprpg.h and the included tprpg, cards, and blackjack examples.
Don't use this for anything security sensitive unless your have someone more
qualified than me evaluate it (and if you do, it would be awsome if you shared
the results).

INSTALLATION
============

Just drop tprpg.c and tprpg.h into your project. This is acceptable even for
closed source commercial projects. See the LICENSE file for details.
