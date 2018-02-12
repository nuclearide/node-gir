const { load } = require('./addon');

module.exports = {
  load,
  get GLib() {
    return require('./GLib');
  },
  get Gtk() {
    return require('./Gtk');
  }
};
