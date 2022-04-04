local mg     = require "moongen"
local memory = require "memory"
local device = require "device"
local ts     = require "timestamping"
local filter = require "filter"
local hist   = require "histogram"
local stats  = require "stats"
local timer  = require "timer"
local log    = require "log"
local pcap   = require "pcap"

function configure(parser)
	parser:description("Generates UDP traffic based on a pcap file."..
     "Searches for the maximum rate that still passes through with <1% loss.")
	parser:argument("txDev", "Device to transmit from."):convert(tonumber)
	parser:argument("rxDev", "Device to receive from."):convert(tonumber)
  parser:option("-f --file", "File to replay."):default("test.pcap")
	parser:option("-s --size", "Packet size."):default(60):convert(tonumber)
	parser:option("-r --rate", "Transmit rate in Mbit/s."):default(10000):convert(tonumber)
	parser:option("-t --timeout", "Time to run the test"):default(0):convert(tonumber)
end

function setRate(queue, packetSize, rate_mbps)
	queue:setRate(rate_mbps - (packetSize + 4) * 8 / 1000);
end

function master(args)
	txDev = device.config{port = args.txDev, rxQueues = 3, txQueues = 3}
	rxDev = device.config{port = args.rxDev, rxQueues = 3, txQueues = 3}
	device.waitForLinks()
	local file = io.open("pcap-replay-results.txt", "w")
	file:write("rate #pkt loss\n")
	local rate = args.rate
	if rate <= 0 then
		rate = 10000
	end

  -- Testing phase
  printf("Test at %d Mbps", rate);
  setRate(txDev:getTxQueue(0), args.size, rate);
  printf("%s",args.file)
  local packetsSent
  local packetsRecv
  local loadTask = mg.startTask("loadSlave", txDev:getTxQueue(0), rxDev,
                                args.file, args.timeout)
  packetsSent, packetsRecv = loadTask:wait()
  local loss = (packetsSent - packetsRecv)/packetsSent
  printf("total: %d rate, %d sent, %f lost",
          rate, packetsSent, loss);
  mg.waitForTasks()

  file:write(rate .. " " .. packetsSent .. " " .. loss .. "\n")
    
end

function loadSlave(queue, rxDev, fname, duration)
	local mempool = memory.createMemPool()
	local bufs = mempool:bufArray()
	local fileTxCtr = stats:newDevTxCounter("txpkts", queue, "CSV", "txpkts.csv")
	local fileRxCtr = stats:newDevRxCounter("rxpkts", rxDev, "CSV", "rxpkts.csv")
	local txCtr = stats:newDevTxCounter(" tx", queue, "nil")
	local rxCtr = stats:newDevRxCounter(" rx", rxDev, "nil")
	local pcapFile = pcap:newReader(fname)
	local finished = timer:new(duration)

  while finished:running() and mg.running() do
    
		local n = pcapFile:read(bufs)	
		if (n == 0) then
			pcapFile:reset()
		end

		queue:sendN(bufs,n)
		txCtr:update()
		fileTxCtr:update()
		rxCtr:update()
		fileRxCtr:update()

  end
  
	txCtr:finalize()
	fileTxCtr:finalize()
	rxCtr:finalize()
	fileRxCtr:finalize()
	pcapFile:close()
	return txCtr.total, rxCtr.total
end
