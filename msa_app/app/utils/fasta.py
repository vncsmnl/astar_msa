"""
FASTA file parsing and analysis utilities
"""


def detect_sequence_type(sequence):
    """Detect if sequence is protein or nucleotide"""
    # Remove whitespace and convert to uppercase
    seq = sequence.upper().replace(' ', '').replace('\n', '')

    if not seq:
        return 'Unknown'

    # Count nucleotide and protein specific characters
    nucleotide_chars = set('ATGCU')
    protein_specific_chars = set('EFILPQZ')

    total_chars = len(seq)
    nucleotide_count = sum(1 for c in seq if c in nucleotide_chars)
    protein_specific_count = sum(1 for c in seq if c in protein_specific_chars)

    # If has protein-specific amino acids, it's definitely protein
    if protein_specific_count > 0:
        return 'Proteína'

    # If more than 95% are nucleotides (A, T, G, C, U, N), likely nucleotide
    if nucleotide_count / total_chars > 0.95:
        return 'Nucleotídeo'

    # Otherwise, likely protein (could have A, T, G, C which are also amino acids)
    return 'Proteína'


def parse_fasta_file(file_path):
    """Read and parse FASTA file, return list of sequences"""
    sequences = []
    current_seq = None

    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('>'):
                if current_seq:
                    sequences.append(current_seq)
                current_seq = {
                    'header': line[1:],
                    'sequence': ''
                }
            elif current_seq is not None:
                current_seq['sequence'] += line

        if current_seq:
            sequences.append(current_seq)

    return sequences


def analyze_sequences(sequences):
    """Calculate statistics and detect sequence types"""
    sequence_types = []
    
    for seq in sequences:
        seq['length'] = len(seq['sequence'])
        seq_type = detect_sequence_type(seq['sequence'])
        seq['type'] = seq_type
        sequence_types.append(seq_type)

    # Determine overall type (most common)
    if sequence_types:
        overall_type = max(set(sequence_types), key=sequence_types.count)
    else:
        overall_type = 'Unknown'

    return overall_type
