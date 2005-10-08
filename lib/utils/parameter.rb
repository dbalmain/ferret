module Ferret::Utils
  class Parameter
    def to_s() return @name end
      
    def _dump(arg)
      @name
    end

    def Parameter._load(var)
      name = var
      key = make_key(name)
      if (@@all_parameters.has_key?(key))
        return @@all_parameters[key]
      else
        return self.new(name)
      end
    end

    def hash
      return self.class.make_key(@name).hash
    end

    protected
      @@all_parameters = {}

      def initialize(name)
        @name = name
        key = self.class.make_key(name)
        
        if (@@all_parameters.has_key?(key))
          raise ArgumentError, "key already in use"
        end

        @@all_parameters[key] = self
      end
    
      def Parameter.make_key(name)
        return self.to_s + " " + name
      end
  end
end
