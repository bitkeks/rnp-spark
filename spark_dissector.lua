-- Spark dissector Lua plugin for Wireshark
-- Built for TU Dresden, RNP 2017 Exercise 2
-- Copyright 2017 Dominik Pataky <dominik.pataky@tu-dresden.de>
-- Licensed under GPLv3, see LICENSE

-- to be run with "wireshark -X lua_script:spark_dissector.lua"

-- create protocol and its fields
p_spark = Proto("spark", "Spark Protocol")
local f_version = ProtoField.uint8("spark.version", "Version", base.HEX)
local f_data_size = ProtoField.uint16("spark.data_size", "Data Size", base.HEX)
local f_data = ProtoField.string("spark.data", "Data", FT_STRING)

p_spark.fields = { f_version, f_data_size, f_data }

-- dissector function
function p_spark.dissector(buf, pkt, root)
  -- validate packet length is adequate, otherwise quit
  if buf:len() == 0 then return end
  pkt.cols.protocol = p_spark.name

  -- create subtree
  subtree = root:add(p_spark, buf(0))
  -- add protocol fields to subtree
  -- buf(start, length)
  subtree:add(f_version, buf(0, 1))
  subtree:add(f_data_size, buf(1, 2))
  subtree:add(f_data, buf(3))
end

-- Initialization routine
function p_spark.init()
end

-- add the protocol dissector to the dissector table and trigger it at type 0x666
local dissector_table = DissectorTable.get("ethertype")
if dissector_table ~= nil then
    dissector_table:add(0x666, p_spark)
end
