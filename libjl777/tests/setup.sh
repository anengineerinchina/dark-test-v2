#!/bin/sh
echo "Installing chRuby (to change current Ruby https://github.com/postmodern/chruby)"

wget -O chruby-0.3.8.tar.gz https://github.com/postmodern/chruby/archive/v0.3.8.tar.gz
tar -xzvf chruby-0.3.8.tar.gz
cd chruby-0.3.8/
sudo make install
rm -R chruby-0.3.8 chruby-0.3.8.tar.gz


echo "Installing RVM (Ruby version manager)"

curl -L https://get.rvm.io | bash -s stable
rvm requirements
rvm install ruby
rvm use ruby --default
[ -f $HOME/.bashrc ] && echo "[[ -s $HOME/.rvm/scripts/rvm ]] && source $HOME/.rvm/scripts/rvm">>$HOME/.bashrc
[ -f $HOME/.zshrc ] && echo "[[ -s $HOME/.rvm/scripts/rvm ]] && source $HOME/.rvm/scripts/rvm">>$HOME/.zshrc
rvm rubygems current

echo "Installing Cucumber (behavioural testing framework) and other libraries required for testing"
bundle install # to automatically install missing gems
gem update

sudo apt-get install expect-dev # Required to unbuffer the output of BitcoinDarkd