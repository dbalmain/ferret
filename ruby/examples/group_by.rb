require 'rubygems'
require 'ferret'

class CatalogueCollector
  class Catalogue
    attr_accessor :score
    attr_reader :name, :docs
    def initialize(name)
      @name = name
      @score = 0.0
      @docs = []
    end

    def <<(doc)
      @docs << doc
      @score += doc[:score]
    end

    def to_s(index)
      @docs = @docs.sort_by{|doc| -doc[:score]}
      artist_width = @docs.collect{|doc| doc[:artist].length}.max
      buf = ["#{@name}: %f" % @score,
             "    Score|Price |Date|%-#{artist_width}s|Piece" % "Artist",
             "   ------|------|----|#{'-' * artist_width}|------------"]
      @docs.each do |doc|
        buf << "   %6.3f|%6s|%4s|%-#{artist_width}s|%s" %
          [doc[:score], doc[:price], doc[:date][0,4], doc[:artist], doc[:work]]
      end
      buf.join("\n")
    end
  end

  def initialize
    @normalized = false
    @catalogues = Hash.new {|h, key| h[key] = Catalogue.new(key)}
  end

  def filter_proc
    lambda do |id, score, searcher|
      doc = searcher[id].load
      age = (Date.today - Date.parse(doc[:date])).to_i
      age_weight = 1 / 2 ** (age.to_f/3650)
      doc[:score] = score * age_weight
      @catalogues[doc[:catalogue]] << doc
      age_weight
    end
  end

  def to_s(index)
    buf = []
    normalize_scores unless @normalized
    @catalogues.keys.sort_by {|key|
      -@catalogues[key].score
    }.each {|key|
      buf << @catalogues[key].to_s(index)
    }
    buf.join("\n\n")
  end
  
  private

  def normalize_scores
    normalization_factor = @catalogues.values.collect {|cat| cat.score}.max
    @catalogues.values.each {|cat| cat.score /= normalization_factor}
    @normalized = true
  end
end

index = Ferret::I.new

art_works = [
  ["$140.0", "No. 5, 1948", "Jackson Pollock", "private sale", "2006-01-30"],
  ["$137.5", "Woman III",  "Willem de Kooning", "private sale", "2006-09-05"],
  ["$135.0", "Portrait of Adele Bloch-Bauer I", "Gustav Klimt", "private sale", "2006-05-31"],
  ["$82.5",  "Portrait of Dr. Gachet",  "Vincent van Gogh", "Christie's, New York", "1990-07-25"],
  ["$78.1",  "Bal au moulin de la Galette, Montmartre",  "Pierre-Auguste Renoir", "Sotheby's, New York", "1990-07-30"],
  ["$104.2", "Garçon à la pipe",  "Pablo Picasso", "Sotheby's, New York", "2005-03-15"],
  ["$53.9",  "Irises",  "Vincent van Gogh", "Sotheby's, New York", "1987-09-12"],
  ["$95.2",  "Dora Maar au Chat",  "Pablo Picasso", "Sotheby's, New York", "2006-08-31"],
  ["$71.5",  "Portrait de l'artiste sans barbe",  "Vincent van Gogh", "Christie's, New York", "1998-01-26"],
  ["$87.9",  "Portrait of Adele Bloch-Bauer II",  "Gustav Klimt", "Christie's, New York", "2006-12-25"],
  ["$76.7",  "Massacre of the Innocents",  "Peter Paul Rubens", "Sotheby's, London", "2002-04-01"],
  ["$49.2",  "Les Noces de Pierrette",  "Pablo Picasso", "Binoche et Godeau Paris", "1985-06-30"],
  ["$80.0",  "False Start",  "Jasper Johns", "private sale", "2006-08-05"],
  ["$57.0",  "A Wheatfield with Cypresses",  "Vincent van Gogh", "private sale", "1993-09-11"],
  ["$47.8",  "Yo, Picasso",  "Pablo Picasso", "Sotheby's, New York", "1989-11-11"],
  ["$60.5",  "Rideau, Cruchon et Compotier",  "Paul Cézanne", "Sotheby's, New York", "1999-04-25"],
  ["$72.8",  "White Center (Yellow, Pink and Lavender on Rose)",  "Mark Rothko", "Sotheby's, New York", "2007-02-28"],
  ["$39.7",  "Vase with Fifteen Sunflowers",  "Vincent van Gogh", "Christie's, London", "1987-03-03"],
  ["$71.7",  "Green Car Crash (Green Burning Car I)",  "Andy Warhol", "Christie's, New York", "2007-08-30"],
  ["$40.7",  "Au Lapin Agile",  "Pablo Picasso", "Sotheby's, New York", "1989-05-20"],
  ["$38.5",  "Acrobate et jeune Arlequin",  "Pablo Picasso", "Christie's, London", "1988-08-08"],
  ["$55.0",  "Femme aux Bras Croisés",  "Pablo Picasso", "Christie's, New York", "2000-09-20"],
  ["$63.5",  "Police Gazette",  "Willem de Kooning", "private sale", "2006-01-13"],
  ["$48.4",  "Le Rêve",  "Pablo Picasso", "Christie's, New York", "1997-06-09"],
  ["$49.6",  "Femme assise dans un jardin",  "Pablo Picasso", "Sotheby's, New York", "1999-09-09"],
  ["$47.5",  "Peasant Woman Against a Background of Wheat",  "Vincent van Gogh", "private sale", "1997-12-01"],
  ["$35.2",  "Portrait of Duke Cosimo I de'Medici", "Pontormo", "Christie's, New York", "1989-08-09"],
].each do |price, work, artist, catalogue, date|
  index << {
    :price      => price,
    :work       => work,
    :artist     => artist,
    :catalogue  => catalogue,
    :date       => date
  }
end

catalogue_collector = CatalogueCollector.new
index.search(ARGV.join(' '), :filter_proc => catalogue_collector.filter_proc)
puts catalogue_collector.to_s(index)
