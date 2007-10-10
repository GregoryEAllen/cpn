#!/usr/bin/env python

import math
import sys

class LFSR:
	def __init__(self,feed,seed):
		self.feed = feed
		self.seed = seed
	def Order(self):
		return int( math.ceil(math.log(self.feed,2)) )
	def MaxVal(self):
		order = self.Order()
		return (2**order) - 1
	def GetResult(self):
		if self.seed & 1:
			self.seed = self.seed>>1 ^ self.feed
		else:
			self.seed = self.seed>>1
		return self.seed


class RandomInstructionGenerator:
	def __init__(self):
		self.lfsr = LFSR(0xF82F,1)
		self.probabilityToCreateNode = 0.01
		self.probabilityToDeleteNode = 0.01

		print "# lfsr of order %d, with range 1-%d" % (self.lfsr.Order(), self.lfsr.MaxVal())

		numNodes = 100
		self.ComputeRanges(numNodes)
		self.currentChain = []
		self.allChains = []
		
		# some (perhaps) interesting stats
		self.numChains = 0
		self.avgChainLength = 0
		self.numNodesCreated = 0
		self.numNodesDeleted = 0
		self.maxNumParallelChains = 0

	def ComputeRanges(self,numNodes):
		self.numNodes = numNodes
		self.createRange = int( round(self.lfsr.MaxVal()*self.probabilityToCreateNode) )
		self.deleteRange = int( round(self.lfsr.MaxVal()*self.probabilityToDeleteNode) )

		print "# createRange = %d, deleteRange = %d" % (self.createRange,self.deleteRange)

		self.chainRange = ((self.lfsr.MaxVal()-self.createRange-self.deleteRange)/self.numNodes) * self.numNodes
		self.noopRange = self.lfsr.MaxVal()-self.createRange-self.deleteRange-self.chainRange

		print "# chainRange = %d, noopRange = %d" % (self.chainRange,self.noopRange)
		totalRange = self.createRange+self.deleteRange + self.chainRange + self.noopRange
#		print "# totalRange = %d" % totalRange

		if self.chainRange==0:
			print "Error: lfsr seed order is too small"
			sys.exit(-1)

	def NumParallelChains(self):
		# (slowly) count the number of chains that could be running in parallel
		# -- this ignores create and delete
		# 1 (a single chain) is not parallel
		if len(self.allChains)<=1: return len(self.allChains)
		maxIdx = len(self.allChains)-1
		chkIdx = len(self.allChains)-2
		numPllChains = 1
		while chkIdx>=0:
			for idx in range(maxIdx,chkIdx,-1):
				#print "## comparing chain %d to %d" % (idx,chkIdx)
				for elem in self.allChains[idx]:
					if self.allChains[chkIdx].count(elem)>0:
						#print "## NumParallelChains: %d is in chains %d and %d" % (elem,idx,chkIdx)
						return numPllChains
			numPllChains = maxIdx-chkIdx+1
			#print "## numPllChains %d" % (numPllChains)
			chkIdx -= 1
		return numPllChains

	def EndCurrentChain(self):
		if len(self.currentChain)<=1:
			print "# discarding a chain of length %d" % len(self.currentChain)
		else:
			avgLen = self.numChains * self.avgChainLength + len(self.currentChain)
			self.allChains.append(self.currentChain[:])
			print "create chain:",
			while len(self.currentChain):
				print self.currentChain.pop(0),
			print ""
			self.numChains += 1
			self.avgChainLength = avgLen * 1. / self.numChains
#			print "avgChainLength %f" % self.avgChainLength
			numParallelChains = self.NumParallelChains()
			print "# numParallelChains %d" % numParallelChains
			if numParallelChains>self.maxNumParallelChains:
				self.maxNumParallelChains = numParallelChains

	def Run(self, sequenceLength = 10):
		idx = 0
		while idx<sequenceLength:
			idx += 1
			prnum = self.lfsr.GetResult()	# pseudo-random number
#			prnum = self.createRange+self.deleteRange+1

			opcode = ''
			arg1 = ''
			
			if (self.createRange>0) & (prnum <= self.createRange):
				opcode = 'create'
				arg1 = self.lfsr.seed
			elif (self.deleteRange>0) & (prnum-self.createRange <= self.deleteRange):
				opcode = 'delete'
				prnum = self.lfsr.GetResult()
				arg1 = (prnum-1) % self.numNodes # slightly biased toward the low end
			elif (prnum-self.createRange-self.deleteRange <= self.chainRange):
				opcode = 'chain'
				arg1 = (prnum-self.createRange-self.deleteRange-1) % self.numNodes
			elif (prnum-self.createRange-self.deleteRange-self.chainRange <= self.noopRange):
				opcode = 'noop'
			else:
				opcode = 'unknown'
		
#			print "#", prnum, opcode, arg1
		
			if opcode == 'chain':
				if self.currentChain.count(arg1)>0:
					# already in the current chain, end the chain
					self.EndCurrentChain()
				else:
					self.currentChain.append(arg1)
			elif opcode == 'create':
				self.EndCurrentChain()
				self.numNodesCreated += 1
			elif opcode == 'delete':
				self.EndCurrentChain()
				self.numNodesDeleted += 1
			#	else:
			#		do nothing
		self.EndCurrentChain()
		
		print "# program statistics"
		print "# numChains %d, avgChainLength %f" % (self.numChains,self.avgChainLength)
		print "# numNodesCreated %d, numNodesDeleted %d" % (self.numNodesCreated,self.numNodesDeleted)
		print "# maxNumParallelChains %d" % (self.maxNumParallelChains)

#		print self.allChains

# the main program
rig = RandomInstructionGenerator()
rig.Run(100000)





	
	

