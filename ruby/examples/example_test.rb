require 'rubygems'
require 'ferret'

index = Ferret::I.new


index << {
  :class_name => 'ArticleTranslation',
  :brand_guid => '123456789',
  :master_brand_guid => '987654321'
}

index.search_each('class_name:ArticleTranslation') do |doc, score|
  puts index[doc][:brand_guid]
  puts index[doc][:master_brand_guid]
end
