= Ferret

Ferret is a Ruby search library inspired by the Apache Lucene search engine for
Java (http://jakarta.apache.org/lucene/). In the same way as Lucene, it is not
a standalone application, but a library you can use to index documents and
search for things in them later.

== Requirements

* Ruby 1.8
* C compiler to build the extension. Tested with gcc, VC6
* make (or nmake on windows)

== Installation

  $ sudo gem install ferret

If you don't have rubygems installed you can still install Ferret. Just
download one of the zipped up versions of Ferret, unzip it and change into the
unzipped directory. Then run the following set of commands;

  $ ruby setup.rb config
  $ ruby setup.rb setup
  $ sudo ruby setup.rb install

== Usage

You can read the TUTORIAL which you'll find in the same directory as this
README. You can also check the following modules for more specific
documentation.

* Ferret::Analysis: for more information on how the data is processed when it
  is tokenized. There are a number of things you can do with your data such as
  adding stop lists or perhaps a porter stemmer. There are also a number of
  analyzers already available and it is almost trivial to create a new one
  with a simple regular expression.

* Ferret::Search: for more information on querying the index. There are a
  number of already available queries and it's unlikely you'll need to create
  your own. You may however want to take advantage of the sorting or filtering
  abilities of Ferret to present your data the best way you see fit.

* Ferret::Document: to find out how to create documents. This part of Ferret
  is relatively straightforward. If you know how Strings, Hashes and Arrays work
  Ferret then you'll be able to create Documents.

* Ferret::QueryParser: if you want to find out more about what you can do with
  Ferret's Query Parser, this is the place to look. The query parser is one
  area that could use a bit of work so please send your suggestions.

* Ferret::Index: for more advanced access to the index you'll probably want to
  use the Ferret::Index::IndexWriter and Ferret::Index::IndexReader. This is
  the place to look for more information on them.

* Ferret::Store: This is the module used to access the actual index storage
  and won't be of much interest to most people.

=== Performance

We are unaware of any alternatives that can out-perform Ferret while still
matching it in features.

== Contact

For bug reports and patches I have set up Trac here;

  http://ferret.davebalmain.com/trac

Queries, discussion etc should be addressed to the mailing lists here;

  http://rubyforge.org/projects/ferret/

Alternatively you could create a new page for discussion on the Ferret wiki;

  http://ferret.davebalmain.com/trac

Of course, since Ferret was ported from Apache Lucene, most of what you can
do with Lucene you can also do with Ferret.

== Authors

[<b>David Balmain</b>] Port to Ruby

[The Apache Software Foundation (Doug Cutting and friends)] Original Apache Lucene

== License

Ferret is available under an MIT-style license.

:include: MIT-LICENSE
