--[[
Description: Probability computation functions for multinomial and nested logit models
Author: Harish Loganathan
]]


function calculate_multinomial_logit_probability(choices, utility, availables)
	local probability = {}
	local evsum = 0
	for k,c in ipairs(choices) do
		probability[c] = availables[c] * math.exp(utility[c])
		evsum = evsum + probability[c]	
	end
	for c in pairs(probability) do
		probability[c] = probability[c]/evsum
	end
	return probability
end

function calculate_nested_logit_probability(choiceset, utility, availables, scales)
	local evmu = {}
	local evsum = {}
	local probability = {}
	for nest,choices in ipairs(choiceset) do
		local mu = scales[nest]
		local nest_evsum = 0
		for i,c in ipairs(choices) do
			local evmuc = availables[c] * math.exp(mu*utility[c])
			evmu[c] = evmuc
			nest_evsum = nest_evsum + evmuc
		end
		evsum[nest] = nest_evsum
	end
		
	sum_evsum_pow_muinv = 0
	for nest,val in pairs(evsum) do
		local mu = scales[nest]
		sum_evsum_pow_muinv = sum_evsum_pow_muinv + math.pow(evsum[nest], (1/mu))
	end

	for nest,choices in ipairs(choiceset) do
		local mu = scales[k]
		for i,c in ipairs(choices) do
			if evsum[nest] != 0 then
				probability[c] = evmu[c] * math.pow(evsum[nest], (1/mu - 1))/sum_evsum_pow_muinv)
			else
				probability[c] = 0
			end
		end
		return probability
	end
end

function calculate_probability(mtype, choiceset, utility, availables, scales)
	local probability = {}
	if mtype == "mnl" then 
		probability = calculate_multinomial_logit_probability(choiceset, utility, availables)
	elseif mtype == "nl" then
		probability = calculate_nested_logit_probability(choiceset,utility,availables,scales)
	else
		error("unknown model type:" .. mtype .. ". Only 'mnl' and 'nl' are currently supported")
	end
	return probability
end

function binary_search(a, x)
	local lo = 0
	local hi = #a
	while lo != hi do
		local mid = math.floor((lo+hi)/2)
		local midval = a[mid]
		if midval > x then 
			hi = mid
		elseif midval < x then
			lo = mid+1
		end
	end
	return hi --or lo since hi == lo is true
end

function make_final_choice(probability)	
	local choices = {}
	local choices_prob = {}
	cum_prob = 0
	for c,p in pairs(probability) do
		table.insert(choices, c)
		cum_prob = cum_prob + p
		table.insert(choices_prob, cum_prob)
	end
	idx = binary_search(choices_prob, math.random()) 
	return choices[idx]
end