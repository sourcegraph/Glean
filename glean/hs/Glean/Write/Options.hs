{-# LANGUAGE ApplicativeDo, NamedFieldPuns #-}
-- Copyright 2004-present Facebook. All Rights Reserved.

module Glean.Write.Options
  ( sendQueueOptions
  , writerOptions
  ) where

import Data.Default (def)
import qualified Options.Applicative as O

import Glean.Write.Async (WriterSettings(..))
import Glean.Write.SendQueue (SendQueueSettings(..))

sendQueueOptions :: O.Parser SendQueueSettings
sendQueueOptions = do
  sendQueueThreads <- O.option O.auto $
    O.long "sender-threads"
    <> O.metavar "N"
    <> O.value 1
    <> O.showDefault
    <> O.help "number of concurrent sender threads, default 1"
  sendQueueMaxMemory <- O.option O.auto $
    O.long "max-send-queue-size"
    <> O.metavar "N"
    <> O.value 2000000000
    <> O.showDefault
    <> O.help "maximum size of send queue (in bytes)"
  sendQueueMaxBatches <- O.option O.auto $
    O.long "max-send-queue-batches"
    <> O.metavar "N"
    <> O.value 1024
    <> O.showDefault
    <> O.help "maximum number of batches in send queue"
  return def{sendQueueThreads, sendQueueMaxMemory, sendQueueMaxBatches}

writerOptions :: O.Parser WriterSettings
writerOptions = do
  writerMaxSize <- O.option O.auto $
    O.long "max-batch-size"
    <> O.metavar "N"
    <> O.value 30000000
    <> O.showDefault
    <> O.help "maximum size of Thrift batch (in bytes)"
  return def{writerMaxSize}