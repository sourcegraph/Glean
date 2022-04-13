{-
  Copyright (c) Meta Platforms, Inc. and affiliates.
  All rights reserved.

  This source code is licensed under the BSD-style license found in the
  LICENSE file in the root directory of this source tree.
-}

module Main ( main ) where

import System.Environment ( withArgs )

import qualified Driver ( main )
import qualified Glean.Regression.Driver.Args.Hack as Hack

main :: IO ()
main = withArgs (Hack.args path) Driver.main
  where
    path = "glean/lang/hack/tests/cases"