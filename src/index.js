const { load } = require('./addon');

module.exports = {
  load,
  get Gtk() {
    return require('./Gtk');
  }
}
