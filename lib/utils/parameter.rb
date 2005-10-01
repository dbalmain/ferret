module Ferret
  module Utils
    # A serializable Enum class.
    class Parameter
      @@all_parameters = {}
      
      def to_s() return @name end
        
      protected
        def initialize(name)
          # typesafe enum pattern, no public constructor
          @name = name
          key = make_key(name)
          
          if (@@all_parameters.has_key?(key))
            raise ArgumentError, "key already in use"
          end

          @@all_parameters[key] = self
        end
      
        def make_key(name)
          return self.class.to_s + " " + name
        end
      
        # Resolves the deserialized instance to the local reference for accurate
        # equals() and == comparisons.
        # 
        # Return a reference to Parameter as resolved in the local VM
        def read_resolve()
          par = @@all_parameters[make_key(@name)]
          
          if(par == nil)
            raise IndexError, "Unknown parameter value " + @name
          end
            
          return par
        end
    end
  end
end
