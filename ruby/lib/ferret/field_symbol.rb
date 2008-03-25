module Ferret
  FIELD_TYPES = %w(integer float string byte).map{|t| t.to_sym}

  class BlankSlate
    instance_methods.each { |m| undef_method m unless m =~ /^__/ }
  end

  module FieldSymbolMethods
    FIELD_TYPES.each do |method|
      define_method(method) do
        fsym = FieldSymbol.new(self, respond_to?(:desc?) ? desc? : false)
        fsym.type = method
        fsym
      end
    end

    def desc
      fsym = FieldSymbol.new(self, respond_to?(:desc?) ? !desc? : true)
      fsym.type = type if respond_to? :type
      fsym
    end

    def desc?
      @desc == true
    end
    def type
      @type || nil
    end
  end

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

class Symbol
  include Ferret::FieldSymbolMethods
end
