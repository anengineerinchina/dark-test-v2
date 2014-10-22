#!/bin/sh
rvm requirements
rvm install ruby
rvm use ruby --default

rvm rubygems current

echo "Installing Cucumber (behavioural testing framework) and other libraries required for testing"
bundle install # to automatically install missing gems
gem update
