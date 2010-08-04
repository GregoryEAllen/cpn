
import sys

wait_policies = [('passive', 'make_data'), ('active', 'make_data'), ('10000', 'make_data_spin')]
stdout = sys.stdout
for i in range(1,17):
    for wait_policy in wait_policies:
        for oper in ['vbf', 'hbf', 'ompbf', 'cpnbf']:
            file = oper + '.' + str(i) + '.' + wait_policy[0]
            stdout.write('data_files += ' + file + '\n')
            stdout.write(file + ':\n')
            stdout.write('\t$(call ' + wait_policy[1] + ',' + oper.upper()+ ',' + wait_policy[0] + ',' + str(i) + ',$@)\n\n')


for action_type in ['avg', 'min', 'max']:
    for wait_policy in wait_policies:
        for operation_type in ['vbf', 'hbf', 'ompbf', 'cpnbf']:
            file = operation_type + '.' + action_type + '.' + wait_policy[0]
            stdout.write(operation_type + '_' + action_type + '_files += ' + file + '\n')
            stdout.write('intermediate_files += ' + file + '\n')
            stdout.write(file + ':')
            for i in range(1,17):
                stdout.write(' ' + operation_type + '.' + str(i) + '.' + wait_policy[0])
            stdout.write('\n')
            stdout.write('\t$(call compute_' + action_type + ',$^,$@)\n\n')

for action_type in ['avg', 'min', 'max']:
        for operation_type in ['ompbf', 'cpnbf']:
            stdout.write('full_' + action_type + '_files += $(' + operation_type + '_' + action_type + '_files)\n')
        for operation_type in ['vbf', 'hbf']:
            stdout.write('half_' + action_type + '_files += $(' + operation_type + '_' + action_type + '_files)\n')

for action_type in ['avg', 'min', 'max']:
    for type in ['full', 'half']:
        file = 'result.' +  type + '.' + action_type + '.%'
        stdout.write(file + ': $(' + type + '_' + action_type + '_files) graph.py\n')
        stdout.write('\tpython graph.py $@ $(' + type + '_' + action_type + '_files)\n\n')
