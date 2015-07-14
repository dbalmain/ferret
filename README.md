Ferret
=======

Ferret is an information retrieval library in the same vein as [Apache Lucene][1].
Originally it was a full port of Lucene but it now uses it's own file format
and indexing algorithm although it is still very similar in many ways to
Lucene. Everything you can do in Lucene you should be able to do in Ferret.

Contents
=========

c/
    Ferret is written in C for speed. The actual C code should be fairly easy
    to use in an application or create bindings to a language other than Ruby.

ruby/
    This directory contains the Ruby bindings and tests. See ruby/README.md for
    information on installing Ferret's Ruby bindings

smoke_test.rb
    If you would like to ensure that ferret keeps working on your system, add
    this method to a cron daemon to run as often as you like. Every time I
    check in a new version of Ferret, this script will pull down the changes
    from the repository and run all of the unit tests (C and Ruby), notifying
    me if anything is broken. The more people running this script the more
    stable ruby will be.

Contributions
=============

To contribute code, please contact me at <dbalmain@gmail.com> or [submit
a ticket][2].

Authors
========

[<b>Dave Balmain</b>](dbalmain@gmail.com)

Acknowledgements
=================

[The Apache Software Foundation (Doug Cutting and friends)][1] Original Apache
Lucene. There have also been many other contributers to Ferret. I will start 
to record them on the Ferret website.

[Jens Krämer][3] for keeping this project alive.

License
========

Ferret is available under an MIT-style license.

See MIT-LICENSE

[1]: (http://lucene.apache.org/core/)
[2]: https://github.com/dbalmain/ferret/issues
[3]: https://github.com/jkraemer/
