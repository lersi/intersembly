#include "keystone_assembler.h" 
#undef __GNUC__
#include <keystone/keystone.h>

using namespace assemble;

static constexpr int COMPATIBLE_KEYSTONE_VERSION = 9;

/**
 * @brief converts our api's arcitecture enum to keystone enum.
 * 
 * @note sets error code.
 * 
 * @param[in] architecture the architecture value to convert.
 * @param[out] keystone_architecture will contain the keystone architecture value if function succeed.
 */
static 
bool 
convert_architecture_to_keystone(
	IN const architecture_e architecture,
	OUT ks_arch & keystone_architecture
){
	bool result = false;
	ks_arch result_architecture = KS_ARCH_MAX;

	switch (architecture)
	{
	case architecture_e::X86:
		result_architecture = KS_ARCH_X86;
		break;
	case architecture_e::ARM:
		result_architecture = KS_ARCH_ARM;
		break;
    case architecture_e::ARM64:
		result_architecture = KS_ARCH_ARM64;
		break;
	case architecture_e::PPC:
		result_architecture = KS_ARCH_PPC;
		break;
	case architecture_e::MIPS:
		result_architecture = KS_ARCH_MIPS;
		break;
	case architecture_e::SPARC:
		result_architecture = KS_ARCH_SPARC;
		break;
	case architecture_e::SYSTEMZ:
		result_architecture = KS_ARCH_SYSTEMZ;
		break;
	case architecture_e::HEXAGON:
		result_architecture = KS_ARCH_HEXAGON;
		break;
	case architecture_e::EVM:
		result_architecture = KS_ARCH_EVM;
		break;
	
	default:
		/* todo: fill error */
		goto cleanup;
	}
	if (!ks_arch_supported(result_architecture))
	{
		/* todo: fill error */
		goto cleanup;
	}

	keystone_architecture = result_architecture;
	result = true;

cleanup:
	return result;
}

/**
 * @brief converts our api's syntax enum to keystone enum.
 *
 * @note sets error code.
 * 
 * @param[in] syntax the assembly syntax value to convert
 * @param[out] keystone_syntax will contain the keystone syntax value if function succeed.
 */
static
bool
convert_syntax_to_keystone(
	IN const assembly_syntax_e syntax,
	OUT ks_opt_value & keystone_syntax 
){
	bool result = false;
	
	switch(syntax) 
	{
		case assembly_syntax_e::INTEL:
			keystone_syntax = KS_OPT_SYNTAX_INTEL;
			break;
		case assembly_syntax_e::ATNT:
			keystone_syntax = KS_OPT_SYNTAX_ATT;
			break;
		default:
			/* @todo: fill error code */
			goto cleanup;
	}

	result = true;

cleanup:
	return result;
}

bool
KeystoneAssembler::is_compatible_with_keystone_abi(){
	return COMPATIBLE_KEYSTONE_VERSION == ks_version(
		NULL /* major version not needed */, 
		NULL /* minor version not needed */
	);
}

KeystoneAssembler::KeystoneAssembler()
: m_keystone_engine(nullptr)
{}

KeystoneAssembler::~KeystoneAssembler(){
	if(nullptr != m_keystone_engine)
	{
		(void)ks_close(reinterpret_cast<ks_engine *>(m_keystone_engine));
		m_keystone_engine = nullptr;
	}
}

bool 
KeystoneAssembler::init(
	IN const architecture_e architecture,
	IN const int architecture_mode,
	IN const assembly_syntax_e syntax
){
	bool result = false;
	ks_err keystone_error_code = KS_ERR_OK;
	ks_arch keystone_architecture;

	if (nullptr != m_keystone_engine) {
		/* @todo: fill error */
		goto cleanup;
	}

	if (!KeystoneAssembler::is_compatible_with_keystone_abi()) {
		/* @todo: fill error */
		goto cleanup;	
	}
	if (!convert_architecture_to_keystone(
		architecture, 
		keystone_architecture)
	){
		goto cleanup;	
	}

	keystone_error_code = ks_open(keystone_architecture, architecture_mode, reinterpret_cast<ks_engine **>(&m_keystone_engine));
	if (KS_ERR_OK != keystone_error_code){
		/* @todo: fill error */
		goto cleanup;
	}

	if(!set_syntax(syntax)) {
		goto cleanup;
	}

	result = true;

cleanup:
	if (!result && nullptr != m_keystone_engine) {
		(void)ks_close(reinterpret_cast<ks_engine *>(m_keystone_engine));
		m_keystone_engine = nullptr;
	}
	return result;
}

bool
KeystoneAssembler::assemble(
	IN const string_t & assembly,
	OUT array_t & opcodes
){
	bool result = false;
	size_t opcodes_count = 0;
	size_t opcodes_length = 0;

	if(0 != ks_asm(
		reinterpret_cast<ks_engine *>(m_keystone_engine),
		reinterpret_cast<const char *>(assembly.str),
		0,
		&opcodes.array,
		&opcodes_length,
		&opcodes_count
	)){
		/* @todo: fill error */
		goto cleanup;
	}

	opcodes.size = static_cast<uint64_t>(opcodes_length);
	result = true;

cleanup:
	return result;
}

bool
KeystoneAssembler::set_syntax(
	IN const assembly_syntax_e syntax
){
	bool result = false;
	ks_err keystone_error_code = KS_ERR_OK;
	ks_opt_value keystone_syntax;

	if (!convert_syntax_to_keystone(
		syntax,
		keystone_syntax
	)){
		goto cleanup;
	}

	keystone_error_code = ks_option(reinterpret_cast<ks_engine *>(m_keystone_engine), KS_OPT_SYNTAX, keystone_syntax);
	if (KS_ERR_OK != keystone_error_code){
		/* @todo: fill error */
		goto cleanup;
	}

	result = true;

cleanup:
	return result;
}

bool
KeystoneAssembler::free(
	IN const uint8_t * opcode_array_to_release
){
	bool result = false;

	if (nullptr == opcode_array_to_release){
		/* @todo: fill error */
		goto cleanup;
	}

	ks_free(
		const_cast<uint8_t *>(opcode_array_to_release)
	);
	result = true;

cleanup:	
	return result;
}
