module Ferret
  FIELD_TYPES = %w(integer float string byte).map{|t| t.to_sym}

  if defined?(BasicObject)
    # Ruby 1.9.x
    class BlankSlate < BasicObject
    end
  else
    # Ruby 1.8.x
    # BlankSlate is a class with no instance methods except for __send__ and
    # __id__. It is useful for creating proxy classes. It is currently used by
    # the FieldSymbol class which is a proxy to the Symbol class 
    class BlankSlate
      instance_methods.each { |m| undef_method m unless m =~ /^__|object_id/ }
    end
  end

  # The FieldSymbolMethods module contains the methods that are added to both
  # the Symbol class and the FieldSymbol class. These methods allow you to set
  # the type easily set the type of a field by calling a method on a symbol.
  #
  # Right now this is only useful for Sorting and grouping, but some day Ferret
  # may have typed fields, in which case these this methods will come in handy.
  #
  # The available types are specified in Ferret::FIELD_TYPES.
  #
  # == Examples
  #
  #   index.search(query, :sort => :title.string.desc)
  #
  #   index.search(query, :sort => [:price.float, :count.integer.desc])
  #   
  #   index.search(query, :group_by => :catalogue.string)
  #
  # == Note
  #
  # If you set the field type multiple times, the last type specified will be
  # the type used. For example;
  #
  #   puts :title.integer.float.byte.string.type.inspect # => :string
  #
  # Calling #desc twice will set desc? to false
  #
  #   puts :title.desc?           # => false
  #   puts :title.desc.desc?      # => true
  #   puts :title.desc.desc.desc? # => false
  module FieldSymbolMethods
    FIELD_TYPES.each do |method|
      define_method(method) do
        fsym = FieldSymbol.new(self, respond_to?(:desc?) ? desc? : false)
        fsym.type = method
        fsym
      end
    end
      
    # Set a field to be a descending field. This only makes sense in sort
    # specifications.
    def desc
      fsym = FieldSymbol.new(self, respond_to?(:desc?) ? !desc? : true)
      fsym.type = type if respond_to? :type
      fsym
    end

    # Return whether or not this field should be a descending field
    def desc?
      @desc == true
    end

    # Return the type of this field
    def type
      @type || nil
    end
  end

  # See FieldSymbolMethods
  class FieldSymbol < BlankSlate
    include FieldSymbolMethods
    def initialize(symbol, desc = false)
      @symbol = symbol
      @desc = desc
    end

    def method_missing(method, *args)
      @symbol.__send__(method, *args)
    end

    attr_writer :type, :desc
  end
end

# See FieldSymbolMethods
class Symbol
  include Ferret::FieldSymbolMethods
end
