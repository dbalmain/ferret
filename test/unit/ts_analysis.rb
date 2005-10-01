$:.unshift File.dirname(__FILE__)
require File.join(File.dirname(__FILE__), "../test_helper.rb")

require 'analysis/tc_letter_tokenizer'
require 'analysis/tc_white_space_tokenizer'
require 'analysis/tc_lower_case_tokenizer'
require 'analysis/tc_word_list_loader'
require 'analysis/tc_lower_case_filter'
require 'analysis/tc_stop_filter'
require 'analysis/tc_porter_stem_filter'
require 'analysis/tc_analyzer'
require 'analysis/tc_stop_analyzer'
require 'analysis/tc_white_space_analyzer'
require 'analysis/tc_per_field_analyzer_wrapper'
require 'analysis/tc_standard_tokenizer'
require 'analysis/tc_standard_analyzer'
