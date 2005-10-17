module Ferret::Search
  # Expert: Scoring API.
  # Subclasses implement search scoring.
  #
  # The score of query *q* for document *d* is defined
  # in terms of these methods as follows:
  #
  # <table cellpadding="0" cellspacing="0" border="0">
  #  <tr>
  #    <td valign="middle" align="right" rowspan="2">score(q,d) =<br></td>
  #    <td valign="middle" align="center">
  #    <big><big><big><big><big>&Sigma</big></big></big></big></big></td>
  #    <td valign="middle"><small>
  #    #tf(int) tf(t in d)#
  #    #idf_term(Term,Searcher) idf(t)#
  #    Field#getBoost getBoost(t.field in d)#
  #    #length_norm(String,int) length_norm(t.field in d)
  #    </small></td>
  #    <td valign="middle" rowspan="2">&nbsp*
  #    #coord(int,int) coord(q,d)#
  #    #query_norm(float) query_norm(q)
  #    </td>
  #  </tr>
  #  <tr>
  #   <td valign="top" align="right">
  #    <small>t in q</small>
  #    </td>
  #  </tr>
  # </table>
  #
  # See #set_default
  # See IndexWriter#set_similarity
  # See Searcher#set_similarity
  class Similarity

    def Similarity.byte_to_float(b)
      if (b == 0)
        return 0.0
      end
      mantissa = b & 0x07           # 0x07 =  7 = 0b00000111
      exponent = (b >> 3) & 0x1F    # 0x1f = 31 = 0b00011111
      return [0,0,(mantissa << 5),(exponent+48)].pack("cccc").unpack("f")[0]
    end

    def Similarity.float_to_byte(f) 
      if (f <= 0.0) then return 0 end

      bits = [f].pack("f").unpack("cccc")
      mantissa = (bits[2] & 0xEf) >> 5 
      exponent = (bits[3] - 48)

      if (exponent > 0x1f)
        exponent = 0x1f   # 0x1f = 31 = 0b00011111
        mantissa = 0x07   # 0x07 =  7 = 0b00000111
      end

      if (exponent < 0)
        exponent = 0
        mantissa = 1
      end

      return ((exponent<<3) | mantissa)
    end

    # Cache of decoded bytes
    NORM_TABLE = Array.new(256) { |i| Similarity.byte_to_float(i) }

    # Decodes a normalization factor stored in an index.
    # See Similarity#encode_norm(float)
    def Similarity.decode_norm(b) 
      return NORM_TABLE[b & 0xFF]
    end

    # Decodes a normalization factor stored in an index.
    # See Similarity#encode_norm(float)
    def decode_norm(b)
      return self.class.decode_norm(b)
    end

    # Computes the normalization value for a field given the total number of
    # terms contained in a field.  These values, together with field boosts, are
    # stored in an index and multipled into scores for hits on each field by the
    # search code.
    #
    # Matches in longer fields are less precise, so implemenations of this
    # method usually return smaller values when *num_tokens* is large,
    # and larger values when *num_tokens* is small.
    #
    # That these values are computed under 
    # IndexWriter#add_document and stored then using
    # #encode_norm(float).  Thus they have limited precision, and documents
    # must be re-indexed if this method is altered.
    #
    # field:: the name of the field
    # num_tokens:: the total number of tokens contained in fields named
    #              _field_ of _doc_.
    #
    # See Field#set_boost
    def length_norm
      raise NotImplementedError
    end

    # Computes the normalization value for a query given the sum of the squared
    # weights of each of the query terms.  This value is then multipled into the
    # weight of each query term.
    #
    # This does not affect ranking, but rather just attempts to make scores
    # from different queries comparable.
    #
    # sum_of_squared_weights:: the sum of the squares of query term weights
    # Return:: a normalization factor for query weights
    def query_norm
      raise NotImplementedError
    end

    # Encodes a normalization factor for storage in an index.
    #
    # The encoding uses a five-bit exponent and three-bit mantissa, thus
    # representing values from around 7x10^9 to 2x10^-9 with about one
    # significant decimal digit of accuracy.  Zero is also represented.
    # Negative numbers are rounded up to zero.  Values too large to represent
    # are rounded down to the largest representable value.  Positive values too
    # small to represent are rounded up to the smallest positive representable
    # value.
    #
    # See Field#boost=
    def Similarity.encode_norm(f) 
      return Similarity.float_to_byte(f)
    end

    def encode_norm(f) 
      return self.class.float_to_byte(f)
    end

    # Computes a score factor based on a term or phrase's frequency in a
    # document.  This value is multiplied by the #idf_term(Term, Searcher)
    # factor for each term in the query and these products are then summed to
    # form the initial score for a document.
    #
    # Terms and phrases repeated in a document indicate the topic of the
    # document, so implementations of this method usually return larger values
    # when _freq_ is large, and smaller values when _freq_
    # is small.
    #
    # The default implementation calls #tf(float)
    #
    # freq:: the frequency of a term within a document
    # Return:: a score factor based on a term's within-document frequency
    def tf
      raise NotImplementedError
    end

    # Computes the amount of a sloppy phrase match, based on an edit distance.
    # This value is summed for each sloppy phrase match in a document to form
    # the frequency that is passed to #tf(float).
    #
    # A phrase match with a small edit distance to a document passage more
    # closely matches the document, so implementations of this method usually
    # return larger values when the edit distance is small and smaller values
    # when it is large.
    #
    # See PhraseQuery#slop(int)
    # distance:: the edit distance of this sloppy phrase match
    # Return:: the frequency increment for this match
    def sloppy_freq
      raise NotImplementedError
    end

    # Computes a score factor for a simple term.
    #
    # The default implementation is:
    #   return idf(searcher.doc_freq(term), searcher.max_doc())
    #
    # Note that Searcher#max_doc() is used instead of
    # IndexReader#num_docs() because it is proportional to
    # Searcher#doc_freq(Term) , i.e., when one is inaccurate,
    # so is the other, and in the same direction.
    #
    # term:: the term in question
    # searcher:: the document collection being searched
    # Return:: a score factor for the term
    def idf_term(term, searcher)
      return idf(searcher.doc_freq(term), searcher.max_doc())
    end

    # Computes a score factor for a phrase.
    #
    # The default implementation sums the #idf(Term,Searcher) factor
    # for each term in the phrase.
    #
    # terms:: the terms in the phrase
    # searcher:: the document collection being searched
    # Return:: a score factor for the phrase
    def idf_phrase(terms, searcher)
      idf = 0.0
      terms.each { |term| idf += idf_term(term, searcher) }
      return idf
    end

    # Computes a score factor based on a term's document frequency (the number
    # of documents which contain the term).  This value is multiplied by the
    # #tf(int) factor for each term in the query and these products are
    # then summed to form the initial score for a document.
    #
    # Terms that occur in fewer documents are better indicators of topic, so
    # implemenations of this method usually return larger values for rare terms,
    # and smaller values for common terms.
    #
    # doc_freq:: the number of documents which contain the term
    # num_docs:: the total number of documents in the collection
    # Return:: a score factor based on the term's document frequency
    def idf
      raise NotImplementedError
    end

    # Computes a score factor based on the fraction of all query terms that a
    # document contains.  This value is multiplied into scores.
    #
    # The presence of a large portion of the query terms indicates a better
    # match with the query, so implemenations of this method usually return
    # larger values when the ratio between these parameters is large and smaller
    # values when the ratio between them is small.
    #
    # overlap:: the number of query terms matched in the document
    # max_overlap:: the total number of terms in the query
    # Return:: a score factor based on term overlap with the query
    def coord
      raise NotImplementedError
    end
  end

  # Expert: Default scoring implementation.
  class DefaultSimilarity < Similarity
    # See source
    def length_norm(field, num_terms)
      return 1.0 / Math.sqrt(num_terms)
    end
    
    # See source
    def query_norm(sum_of_squared_weights)
      return 1.0 / Math.sqrt(sum_of_squared_weights)
    end

    # See source
    def tf(freq)
      return Math.sqrt(freq)
    end
      
    # See source
    def sloppy_freq(distance)
      return 1.0 / (distance + 1)
    end
      
    # See source
    def idf(doc_freq, num_docs)
      return 0.0 if num_docs == 0
      return Math.log(num_docs.to_f/(doc_freq+1)) + 1.0
    end
      
    # See source
    def coord(overlap, max_overlap)
      return overlap.to_f / max_overlap
    end
  end

  class Similarity
    # The Similarity implementation used by default.
    @@default = DefaultSimilarity.new()

    def Similarity.default
      return @@default
    end

    def Similarity.default=(default)
      @@default = default
    end
  end
end
