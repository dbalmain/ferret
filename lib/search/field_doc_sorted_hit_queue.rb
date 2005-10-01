# Expert: Collects sorted results from Searchable's and collates them.
# The elements put into this queue must be of type FieldDoc.
class FieldDocSortedHitQueue PriorityQueue 

	# this cannot contain AUTO fields - any AUTO fields should
	# have been resolved by the time this class is used.
	volatile SortField[] fields

	# used in the case where the fields are sorted by locale
	# based strings
	volatile Collator[] collators


	# Creates a hit queue sorted by the given list of fields.
	# fields:: Field names, in priority order (highest priority first).
	# size::  The number of hits to retain.  Must be greater than zero.
	FieldDocSortedHitQueue (fields, size) 
		@fields = fields
		@collators = hasCollators (fields)
		initialize (size)
	end


	# Allows redefinition of sort fields if they are +nil+.
	# This is to handle the case using ParallelMultiSearcher where the
	# original list contains AUTO and we don't know the actual sort
	# type until the values come back.  The fields can only be set once.
	# This method is thread safe.
	# fields::
	synchronized void setFields (fields) 
		if (@fields == nil) 
			@fields = fields
			@collators = hasCollators (fields)
		end
	end


	# Returns the fields being used to sort. 
	SortField[] getFields() 
		return fields
	end


	# Returns an array of collators, possibly +nil+.  The collators
	# correspond to any SortFields which were given a specific locale.
	# fields:: Array of sort fields.
	# returns:: Array, possibly +nil+.
	private Collator[] hasCollators (final SortField[] fields) 
		if (fields == nil) return nil
		Collator[] ret = new Collator[fields.length]
		for (int i=0; i<fields.length; ++i) 
			Locale locale = fields[i].getLocale()
			if (locale != nil)
				ret[i] = Collator.getInstance (locale)
		end
		return ret
	end


	# Returns whether +a+ is less relevant than +b+.
	# a:: ScoreDoc
	# b:: ScoreDoc
	# returns:: +true+ if document +a+ should be sorted after document +b+.
	protected final boolean lessThan (final Object a, final Object b) 
		final FieldDoc docA = (FieldDoc) a
		final FieldDoc docB = (FieldDoc) b
		 n = fields.length
		def c = 0
		for (int i=0; i<n and c==0; ++i) 
			def type = fields[i].getType()
			if (fields[i].getReverse()) 
				switch (type) 
					case SortField.SCORE:
						float r1 = ((Float)docA.fields[i]).floatValue()
						float r2 = ((Float)docB.fields[i]).floatValue()
						if (r1 < r2) c = -1
						if (r1 > r2) c = 1
						break
					case SortField.DOC:
					case SortField.INT:
						def i1 = ((Integer)docA.fields[i]).intValue()
						def i2 = ((Integer)docB.fields[i]).intValue()
						if (i1 > i2) c = -1
						if (i1 < i2) c = 1
						break
					case SortField.STRING:
						def s1 = (String) docA.fields[i]
						def s2 = (String) docB.fields[i]
						if (s2 == nil) c = -1;      # could be nil if there are
						elsif (s1 == nil) c = 1;  # no terms in the given field
						elsif (fields[i].getLocale() == nil) 
							c = s2.compareTo(s1)
						else 
							c = collators[i].compare (s2, s1)
						end
						break
					case SortField.FLOAT:
						float f1 = ((Float)docA.fields[i]).floatValue()
						float f2 = ((Float)docB.fields[i]).floatValue()
						if (f1 > f2) c = -1
						if (f1 < f2) c = 1
						break
					case SortField.CUSTOM:
						c = docB.fields[i].compareTo (docA.fields[i])
						break
					case SortField.AUTO:
						# we cannot handle this - even if we determine the type of object (Float or
						# Integer), we don't necessarily know how to compare them (both SCORE and
						# FLOAT both contain floats, but are sorted opposite of each other). Before
						# we get here, each AUTO should have been replaced with its actual value.
						raise new RuntimeException ("FieldDocSortedHitQueue cannot use an AUTO SortField")
					default:
						raise new RuntimeException ("invalid SortField type: "+type)
				end
			else 
				switch (type) 
					case SortField.SCORE:
						float r1 = ((Float)docA.fields[i]).floatValue()
						float r2 = ((Float)docB.fields[i]).floatValue()
						if (r1 > r2) c = -1
						if (r1 < r2) c = 1
						break
					case SortField.DOC:
					case SortField.INT:
						def i1 = ((Integer)docA.fields[i]).intValue()
						def i2 = ((Integer)docB.fields[i]).intValue()
						if (i1 < i2) c = -1
						if (i1 > i2) c = 1
						break
					case SortField.STRING:
						def s1 = (String) docA.fields[i]
						def s2 = (String) docB.fields[i]
						# nil values need to be sorted first, because of how FieldCache.getStringIndex()
						# works - in that routine, any documents without a value in the given field are
						# put first.
						if (s1 == nil) c = -1;      # could be nil if there are
						elsif (s2 == nil) c = 1;  # no terms in the given field
						elsif (fields[i].getLocale() == nil) 
							c = s1.compareTo(s2)
						else 
							c = collators[i].compare (s1, s2)
						end
						break
					case SortField.FLOAT:
						float f1 = ((Float)docA.fields[i]).floatValue()
						float f2 = ((Float)docB.fields[i]).floatValue()
						if (f1 < f2) c = -1
						if (f1 > f2) c = 1
						break
					case SortField.CUSTOM:
						c = docA.fields[i].compareTo (docB.fields[i])
						break
					case SortField.AUTO:
						# we cannot handle this - even if we determine the type of object (Float or
						# Integer), we don't necessarily know how to compare them (both SCORE and
						# FLOAT both contain floats, but are sorted opposite of each other). Before
						# we get here, each AUTO should have been replaced with its actual value.
						raise new RuntimeException ("FieldDocSortedHitQueue cannot use an AUTO SortField")
					default:
						raise new RuntimeException ("invalid SortField type: "+type)
				end
			end
		end
		return c > 0
	end
end
