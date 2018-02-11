try {
  // attempt to load the native module from a debug build first
  // eslint-disable-next-line import/no-unresolved
  module.exports = require('../build/Debug/girepository');
} catch (error) {
  if (error.code !== 'MODULE_NOT_FOUND') {
    throw error;
  }
  // if the debug build couldn't be found, then attempt
  // to load the release build.
  // don't catch errors for any require() failures on the
  // release build, we should fail fast!
  // eslint-disable-next-line import/no-unresolved
  module.exports = require('../build/Release/girepository');
}
